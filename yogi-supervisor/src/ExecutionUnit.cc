#include "ExecutionUnit.hh"

#include <boost/algorithm/string.hpp>

#include <istream>
#include <algorithm>
using namespace std::string_literals;


ExecutionUnit::ExecutionUnit(boost::asio::io_service& ios, FileWatcher& fileWatcher, std::string name,
    const yogi::ConfigurationChild& configChild, std::string defaultsConfigChildName,
    const template_string_vector& constants)
: m_ios(ios)
, m_fileWatcher(fileWatcher)
, m_name(name)
, m_configChild(configChild)
, m_defaultsConfigChild(yogi::ProcessInterface::config().get_child(defaultsConfigChildName))
, m_constants(constants)
{
    read_configuration();
    create_variables();
}

ExecutionUnit::~ExecutionUnit()
{
    if (m_startupCommand) {
        m_startupCommand->kill();
    }
}

const std::string& ExecutionUnit::name() const
{
    return m_name;
}

void ExecutionUnit::start()
{
    if (!m_enabled) {
        return;
    }

    check_command_not_empty("execution-command");
    for (auto& file : m_filesTriggeringRestart) {
        file.resolve(m_variables);
        YOGI_LOG_INFO("Execution unit " << m_name << " will be restarted on changes to " << file);
    }

    run_command(m_startupCommand, m_variables, [=](auto exitStatus, auto& out, auto& err) {
        if (exitStatus == Command::SUCCESS) {
            this->on_startup_command_finished_successfully();
            this->start_watching_files();
        }
        else {
            std::ostringstream oss;
            oss << "Startup command for " << m_name << " exited with status " << exitStatus;
            throw std::runtime_error(oss.str());
        }
    });
}

boost::asio::io_service& ExecutionUnit::io_service()
{
    return m_ios;
}

const template_string_vector& ExecutionUnit::variables() const
{
    return m_variables;
}

const TemplateString& ExecutionUnit::logfile() const
{
    return m_logfile;
}

const std::chrono::milliseconds ExecutionUnit::restart_delay() const
{
    return m_restartDelay;
}

FileWatcher& ExecutionUnit::file_watcher()
{
    return m_fileWatcher;
}

void ExecutionUnit::set_variable(const std::string& name, const std::string& value)
{
    for (auto& variable : m_variables) {
        if (variable.name() == name) {
            variable = TemplateString(name, value);
            return;
        }
    }

    m_variables.push_back(TemplateString(name, value));
}

bool ExecutionUnit::extract_bool(const std::string& childName)
{
    return m_configChild.get<bool>(childName, m_defaultsConfigChild.get<bool>(childName));
}

std::chrono::milliseconds ExecutionUnit::extract_duration(const std::string& childName)
{
    auto val = m_configChild.get<float>(childName, m_defaultsConfigChild.get<float>(childName));
    auto ms = static_cast<int>(val * 1000);

    if (ms == -1) {
        return std::chrono::milliseconds::max();
    }

    if (ms < 100) {
        throw std::runtime_error("Invalid value "s + std::to_string(val) + " in " + m_name + "." + childName
            + ". The minimum value is 0.1. Use -1 for infinity.");
    }

    return std::chrono::milliseconds(ms);
}

command_ptr ExecutionUnit::extract_command(const std::string& childName, std::chrono::milliseconds timeout)
{
    auto cmdName = childName + " command for " + m_name;
    auto ts = extract_string(childName);

    return std::make_shared<Command>(m_ios, cmdName, timeout, ts);
}

void ExecutionUnit::check_command_not_empty(const std::string& cmdName)
{
    auto ts = extract_string(cmdName);
    if (ts.value().empty()) {
        std::ostringstream oss;
        oss << "Command " << *this << "." << cmdName << " is not set";
        throw std::runtime_error(oss.str());
    }
}

TemplateString ExecutionUnit::extract_string(const std::string& childName)
{
    auto val = m_configChild.get<std::string>(childName, m_defaultsConfigChild.get<std::string>(childName, ""));
    auto name = childName.rfind('.') == std::string::npos ? childName : childName.substr(childName.rfind('.') + 1);
    auto ts = TemplateString(name, val);
    ts.resolve(m_constants);
    return ts;
}

void ExecutionUnit::run_command(const command_ptr& cmd, const template_string_vector& variables,
    std::function<void (Command::exit_status_t, const std::string&, const std::string&)> completionHandler,
    bool logFailureAsError)
{
    if (cmd->empty()) {
        YOGI_LOG_TRACE("Skipping " << *cmd << " since the command is unset");
        completionHandler(Command::SUCCESS, {}, {});
        return;
    }

    cmd->async_run(variables, [=, cmd = cmd.get()](auto exitStatus, auto&& out, auto&& err) {
        if (exitStatus == Command::SUCCESS) {
            YOGI_LOG_DEBUG("Successfully executed " << *cmd);
            log_command_output(out, err, false);
        }
        else {
            if (!logFailureAsError && exitStatus == Command::FAILURE) {
                YOGI_LOG_DEBUG("Command " << *cmd << " exited with an error code");
            }
            else {
                YOGI_LOG_ERROR("Error while executing " << *cmd << ": " << exitStatus);
            }

            log_command_output(out, err, logFailureAsError);
        }

        completionHandler(exitStatus, out, err);
    });
}

void ExecutionUnit::log_command_output(const std::string& out, const std::string& err, bool logAsError)
{
    auto fn = [&](auto& type, auto& text) {
        std::vector<std::string> lines;
        if (!text.empty()) {
            boost::split(lines, text, boost::is_any_of("\n"));
        }

        std::ostringstream oss;
        oss << "Command " << type << " (" << lines.size() << " lines):";
        if (logAsError) {
            YOGI_LOG_ERROR(oss.str());
        }
        else {
            YOGI_LOG_TRACE(oss.str());
        }

        for (auto& line : lines) {
            if (logAsError) {
                YOGI_LOG_ERROR("#> " << line);
            }
            else {
                YOGI_LOG_TRACE("#> " << line);
            }
        }
    };

    fn("stdout", out);
    fn("stderr", err);
}

void ExecutionUnit::start_timer(boost::asio::deadline_timer& timer, std::chrono::milliseconds duration,
    std::function<void ()> successHandler)
{
    if (duration != duration.max()) {
        timer.expires_from_now(boost::posix_time::milliseconds(duration.count()));
        timer.async_wait([=](auto& ec) {
            if (!ec) {
                successHandler();
            }
        });
    }
}

void ExecutionUnit::read_configuration()
{
    m_enabled = extract_bool("enabled");

    if (m_enabled) {
        YOGI_LOG_DEBUG("Found enabled execution unit " << m_name);
        check_command_not_empty("execution-command");
    }
    else {
        YOGI_LOG_DEBUG("Found disabled execution unit " << m_name);
    }

    m_logfile = extract_string("logfile");
    m_startupTimeout = extract_duration("startup-timeout");
    m_startupCommand = extract_command("startup-command", m_startupTimeout);
    m_restartDelay = extract_duration("restart-delay");

    extract_files_triggering_restart();
}

void ExecutionUnit::create_variables()
{
}

void ExecutionUnit::extract_files_triggering_restart()
{
    const char* childName = "files-triggering-restart";
    for (auto child : m_configChild.get_child(childName, m_defaultsConfigChild.get_child(childName))) {
        auto file = child.second.get_value<std::string>();
        auto ts = TemplateString(file);
        ts.resolve(m_constants);
        m_filesTriggeringRestart.push_back(ts);
    }
}

void ExecutionUnit::start_watching_files()
{
    for (auto& ts : m_filesTriggeringRestart) {
        m_fileWatcher.watch(ts.value(), [=](auto& filename, auto eventType) {
            this->on_watched_file_changed();
        });
    }
}

std::ostream& operator<< (std::ostream& os, const ExecutionUnit& group)
{
    return os << group.name();
}
