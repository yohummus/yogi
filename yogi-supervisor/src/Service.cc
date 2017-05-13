#include "Service.hh"

Service::Service(boost::asio::io_service& ios, std::string name,
    const yogi::ConfigurationChild& configChild, const template_string_vector& constants)
: ExecutionUnit(ios, name, configChild, "defaults.services", constants)
, m_restartTimer(ios)
{
    read_configuration();
    set_variable("SERVICE", name);
}

void Service::on_startup_command_finished_successfully()
{
    run_execution_command();
}

void Service::read_configuration()
{
    m_restartDelay = extract_duration("restart-delay");
    m_executionCommand = extract_command("execution-command", std::chrono::milliseconds::max());
    check_command_not_empty("execution-command");
}

void Service::run_execution_command()
{
    YOGI_LOG_INFO("Starting execution command for " << name() << "...");
    m_executionCommand->async_run(variables(), [=](auto exitStatus, auto& out, auto& err) {
       this->on_execution_command_finished(exitStatus);
    });
}

void Service::on_execution_command_finished(Command::exit_status_t exitStatus)
{
    YOGI_LOG_WARNING("Execution command for " << name() << " finished with status " << exitStatus);

    if (m_restartDelay != m_restartDelay.max()) {
        m_restartTimer.expires_from_now(boost::posix_time::milliseconds(m_restartDelay.count()));
        m_restartTimer.async_wait([=](auto& ec) {
            if (!ec) {
                this->run_execution_command();
            }
        });
    }
}
