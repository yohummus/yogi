#include "Service.hh"

Service::Service(boost::asio::io_service& ios, std::string name,
    const yogi::ConfigurationChild& configChild, const template_string_vector& constants)
: ExecutionUnit(ios, name, configChild, "defaults.services", constants)
{
    read_configuration();
    set_variable("SERVICE", name);
}

void Service::on_startup_command_finished_successfully()
{

}

void Service::read_configuration()
{
    m_restartDelay = extract_duration("restart-delay");
    check_command_not_empty("execution-command");
}
