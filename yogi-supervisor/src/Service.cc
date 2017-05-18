#include "Service.hh"

Service::Service(boost::asio::io_service& ios, FileWatcher& fileWatcher, std::string name,
    const yogi::ConfigurationChild& configChild, const template_string_vector& constants)
: ExecutionUnit(ios, fileWatcher, name, configChild, "defaults.services", constants)
, m_restartTimer(ios)
{
    read_configuration();
    set_variable("SERVICE", name);
}

Service::~Service()
{
    if (m_executionCommand) {
        m_executionCommand->kill();
    }
}

void Service::on_startup_command_finished_successfully()
{
    run_execution_command();
}

void Service::on_watched_file_changed()
{
    YOGI_LOG_INFO("Restarting execution command for " << name() << " due to file change");
    m_executionCommand->kill();
    run_execution_command();
}

void Service::read_configuration()
{
}

void Service::run_execution_command()
{
    YOGI_LOG_INFO("Starting execution command for " << name() << "...");
    m_executionCommand = extract_command("execution-command", std::chrono::milliseconds::max());
    m_executionCommand->async_run(variables(), logfile(), [=](auto exitStatus, auto&& out, auto&& err) {
       this->on_execution_command_finished(exitStatus);
    });
}

void Service::on_execution_command_finished(Command::exit_status_t exitStatus)
{
    if (exitStatus == Command::KILLED) {
        YOGI_LOG_TRACE("Execution command for " << name() << " finished with status " << exitStatus);
    }
    else {
        YOGI_LOG_WARNING("Execution command for " << name() << " finished with status " << exitStatus);
        start_restart_timer();
    }
}

void Service::start_restart_timer()
{
    start_timer(m_restartTimer, restart_delay(), [=] {
        this->on_restart_timer_timed_out();
    });
}

void Service::on_restart_timer_timed_out()
{
    run_execution_command();
}
