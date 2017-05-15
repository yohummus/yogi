#include "Command.hh"

#include <yogi.hpp>

#include <boost/asio/read.hpp>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
using namespace std::string_literals;


Command::Command(boost::asio::io_service& ios, std::string name, std::chrono::milliseconds timeout,
    TemplateString cmd)
: m_ios(ios)
, m_signals(ios, SIGCHLD)
, m_timer(ios)
, m_name(name)
, m_timeout(timeout)
, m_timedOut(false)
, m_killed(false)
, m_cmd(cmd)
, m_childPid(0)
, m_outPipeReadSd(ios)
, m_outPipeWriteSd(ios)
, m_errPipeReadSd(ios)
, m_errPipeWriteSd(ios)
, m_childOutBuffer(250)
, m_childErrBuffer(250)
{
    create_pipe(&m_outPipeReadSd, &m_outPipeWriteSd);
    create_pipe(&m_errPipeReadSd, &m_errPipeWriteSd);
}

Command::~Command()
{
}

const std::string& Command::name() const
{
    return m_name;
}

bool Command::empty() const
{
    return m_cmd.value().empty();
}

void Command::async_run(const template_string_vector& variables, TemplateString logfile, completion_handler_fn fn)
{
    if (m_cmd.value().empty()) {
        throw std::runtime_error("Cannot run empty command");
    }

    if (m_logfile.is_open()) {
        m_logfile.close();
    }

    if (!logfile.value().empty()) {
        logfile.resolve(variables);
        m_logfile.open(logfile.value(), std::ostream::out | std::ofstream::trunc);
        if (m_logfile.is_open()) {
            YOGI_LOG_DEBUG("Logging output of " << *this << " to " << logfile.value());
        }
        else {
            YOGI_LOG_WARNING("Could not open/create " << logfile.value());
        }
    }

    m_ios.notify_fork(boost::asio::io_service::fork_prepare);

    m_childPid = fork();
    if (m_childPid == -1) {
        YOGI_LOG_ERROR("Could not fork() for " << *this);
        m_ios.post([=] {
            fn(STARTUP_FAILURE, {}, {});
        });
        return;
    }

    // child process
    if (m_childPid == 0) {
        m_ios.notify_fork(boost::asio::io_service::fork_child);
        YOGI_LOG_TRACE("Child process for " << *this << " started");

        auto cmd = m_cmd;
        cmd.resolve(variables);
        YOGI_LOG_DEBUG("Executing " << *this << ": /bin/sh -c " << cmd.value());

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        dup2(m_outPipeWriteSd.native_handle(), STDOUT_FILENO);
        dup2(m_errPipeWriteSd.native_handle(), STDERR_FILENO);
        close_pipe_sds();

        setsid(); // create process group so we can kill all children that this command spawned via kill(-m_childPid, SIGTERM)
        execute_child_process(cmd.value());
    }
    // parent process
    else {
        m_ios.notify_fork(boost::asio::io_service::fork_parent);
        YOGI_LOG_DEBUG("Started " << *this << " with child PID " << m_childPid);

        m_completionHandler = fn;
        start_child_monitoring();
    }
}

void Command::async_run(const template_string_vector& variables, completion_handler_fn fn)
{
    async_run(variables, TemplateString(), fn);
}

void Command::async_run(completion_handler_fn fn)
{
    async_run({}, fn);
}

void Command::kill()
{
    kill_child();
    m_timer.cancel();
    m_outPipeReadSd.cancel();
    m_errPipeReadSd.cancel();
}

void Command::create_pipe(boost::asio::posix::stream_descriptor* readSd,
    boost::asio::posix::stream_descriptor* writeSd)
{
    int fds[2];
    if (pipe(fds) == -1) {
        std::ostringstream oss;
        oss << "Could not create pipe for "s << *this;
        throw std::runtime_error(oss.str());
    }

    readSd->assign(fds[0]);
    writeSd->assign(fds[1]);
}

void Command::close_pipe_sds()
{
    m_outPipeReadSd.close();
    m_outPipeWriteSd.close();
    m_errPipeReadSd.close();
    m_errPipeWriteSd.close();
}

void Command::execute_child_process(const std::string& command)
{
    char* argv[4];
    argv[0] = new char[8];
    strcpy(argv[0], "/bin/sh");
    argv[1] = new char[3];
    strcpy(argv[1], "-c");
    argv[2] = new char[command.size() + 1];
    strcpy(argv[2], command.c_str());
    argv[3] = 0;

    execv(argv[0], argv);
}

void Command::start_child_monitoring()
{
    m_childOut.clear();
    m_childErr.clear();
    m_timedOut = false;

    async_read_all(&m_outPipeReadSd, &m_childOutBuffer, &m_childOut);
    async_read_all(&m_errPipeReadSd, &m_childErrBuffer, &m_childErr);
    async_await_timeout();
    async_await_signal();
}

void Command::async_read_all(boost::asio::posix::stream_descriptor* sd, std::vector<char>* buf, std::string* str)
{
    auto self = shared_from_this();
    sd->async_read_some(boost::asio::buffer(*buf), [=, self=self](auto& ec, auto bytesRead) {
        if (!ec) {
            if (m_logfile.is_open()) {
                m_logfile.write(buf->data(), buf->size());
                m_logfile.flush();
            }

            str->append(buf->begin(), buf->begin() + bytesRead);
            this->async_read_all(sd, buf, str);
        }
    });
}

void Command::async_await_signal()
{
    auto self = shared_from_this();
    m_signals.async_wait([=, self=self](auto& ec, int sig) {
        if (ec) {
            return;
        }

        int status;
        if (waitpid(m_childPid, &status, WNOHANG) == m_childPid) {
            YOGI_LOG_TRACE("Child process for " << *this << " terminated");

            exit_status_t es = UNKNOWN;
            if (WIFEXITED(status)) {
                es = WEXITSTATUS(status) == 0 ? SUCCESS : FAILURE;
            }

            if (WIFSIGNALED(status)) {
                es = m_timedOut ? TIMEOUT : CANCELED;
            }

            if (m_killed && !m_timedOut) {
                es = KILLED;
            }

            if (es == UNKNOWN) {
                this->async_await_signal();
            }
            else {
                m_childPid = 0;
                m_timer.cancel();

                auto fn = m_completionHandler;
                auto out = m_childOut;
                auto err = m_childErr;
                m_ios.post([=]{
                    fn(es, out, err);
                });
            }
        }
        else {
            this->async_await_signal();
        }
    });
}

void Command::async_await_timeout()
{
    if (m_timeout == std::chrono::milliseconds::max()) {
        return;
    }

    m_timer.expires_from_now(boost::posix_time::milliseconds(m_timeout.count()));

    auto self = shared_from_this();
    m_timer.async_wait([=, self=self](auto& ec) {
        if (!ec) {
            YOGI_LOG_WARNING("Process for " << *this << " timed out");
            this->kill_child();
            m_timedOut = true;
        }
    });
}

void Command::kill_child()
{
    if (m_childPid && !m_killed) {
        if (::kill(-m_childPid, SIGTERM) == -1) {
            YOGI_LOG_TRACE("Could not kill process (PID: " << m_childPid << ") for " << *this << ". Trying again...");
            
            auto self = shared_from_this();
            m_ios.post([=, self=self] {
                this->kill_child();
            });
        }

        m_killed = true;
    }
}

std::ostream& operator<< (std::ostream& os, const Command& cmd)
{
    return os << cmd.name();
}

std::ostream& operator<< (std::ostream& os, const Command::exit_status_t& status)
{
    const char* str;
    switch (status) {
    case Command::UNKNOWN:
        str = "UNKNOWN";
        break;

    case Command::SUCCESS:
        str = "SUCCESS";
        break;

    case Command::FAILURE:
        str = "FAILURE";
        break;

    case Command::TIMEOUT:
        str = "TIMEOUT";
        break;

    case Command::CANCELED:
        str = "CANCELED";
        break;

    case Command::KILLED:
        str = "KILLED";
        break;

    case Command::STARTUP_FAILURE:
        str = "STARTUP_FAILURE";
        break;

    default:
        str = "INVALID";
        break;
    }
    return os << str;
}
