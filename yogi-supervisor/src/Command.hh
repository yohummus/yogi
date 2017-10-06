#ifndef COMMAND_HH
#define COMMAND_HH

#include "TemplateString.hh"

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/optional.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <fstream>


class Command final : public std::enable_shared_from_this<Command>
{
public:
    enum exit_status_t {
        UNKNOWN,
        SUCCESS,
        FAILURE,
        TIMEOUT,
        CANCELED,
        KILLED,
        STARTUP_FAILURE
    };

    typedef std::function<void (exit_status_t, std::string&& out, std::string&& err)> completion_handler_fn;

    Command(boost::asio::io_service& ios, std::string name, std::chrono::milliseconds timeout, TemplateString cmd);
    virtual ~Command();

    const std::string& name() const;
    bool empty() const;

    void async_run(const template_string_vector& variables, boost::optional<TemplateString> logfile,
        completion_handler_fn fn);
    void async_run(const template_string_vector& variables, completion_handler_fn fn);
    void async_run(completion_handler_fn fn);
    void kill();

private:
    void create_pipe(boost::asio::posix::stream_descriptor* readSd,
        boost::asio::posix::stream_descriptor* writeSd);
    void close_pipe_sds();
    void execute_child_process(const std::string& command);
    void start_child_monitoring();
    void async_read_line(boost::asio::posix::stream_descriptor* sd, boost::asio::streambuf* sb);
    void async_await_signal();
    void async_await_timeout();
    void kill_child();

    boost::asio::io_service&              m_ios;
    boost::asio::signal_set               m_signals;
    boost::asio::deadline_timer           m_timer;
    std::string                           m_name;
    std::chrono::milliseconds             m_timeout;
    bool                                  m_timedOut;
    bool                                  m_killed;
    TemplateString                        m_cmd;
    int                                   m_childPid;
    boost::asio::posix::stream_descriptor m_outPipeReadSd;
    boost::asio::posix::stream_descriptor m_outPipeWriteSd;
    boost::asio::posix::stream_descriptor m_errPipeReadSd;
    boost::asio::posix::stream_descriptor m_errPipeWriteSd;
    boost::asio::streambuf                m_childOut;
    boost::asio::streambuf                m_childErr;
    completion_handler_fn                 m_completionHandler;
    std::ofstream                         m_logfile;
    bool                                  m_discardOutput;
};

std::ostream& operator<< (std::ostream& os, const Command& cmd);
std::ostream& operator<< (std::ostream& os, const Command::exit_status_t& status);

typedef std::shared_ptr<Command> command_ptr;

#endif // COMMAND_HH
