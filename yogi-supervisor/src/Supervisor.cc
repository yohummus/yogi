#include "Supervisor.hh"
#include "Service.hh"
#include "DeviceGroup.hh"

using namespace std::string_literals;


Supervisor::Supervisor()
: m_work(m_ios)
, m_signals(m_ios, SIGINT, SIGTERM)
, m_fileWatcher(m_ios)
{
    extract_constants();
    extract_execution_units();
}

void Supervisor::run()
{
    start_waiting_for_termination_signals();
    start_execution_units();

    m_ios.run();
}

void Supervisor::extract_constants()
{
    auto rawConstants = extract_raw_constants();
    for (auto itA = rawConstants.rbegin(); itA != rawConstants.rend(); ++itA) {
        for (auto itB = itA + 1; itB != rawConstants.rend(); ++itB) {
            itA->resolve(*itB);
        }
    }

    for (auto& templateString : rawConstants) {
        m_constants.push_back(templateString);
        YOGI_LOG_TRACE("Loaded constant " << templateString);
    }

    YOGI_LOG_DEBUG("Loaded " << m_constants.size() << " constants from the configuration");
}

template_string_vector Supervisor::extract_raw_constants()
{
    template_string_vector rawConstants;

    auto constantsChild = yogi::ProcessInterface::config().get_child("constants");
    for (auto child : constantsChild) {
        auto name = child.first;
        auto value = child.second.get_value<std::string>();

        if (name == "DEVICE" || name == "DEVICE_NAME" || name == "HOSTNAME") {
            throw std::runtime_error("Invalid constant name "s + name + ". This is a reserved name.");
        }

        rawConstants.push_back(TemplateString(name, value));
    }

    return rawConstants;
}

void Supervisor::extract_execution_units()
{
    auto servicesChild = yogi::ProcessInterface::config().get_child("services");
    for (auto child : servicesChild) {
        m_executionUnits.emplace_back(std::make_unique<Service>(m_ios, m_fileWatcher, child.first, child.second,
            m_constants));
    }

    auto deviceGroupsChild = yogi::ProcessInterface::config().get_child("device-groups");
    for (auto child : deviceGroupsChild) {
        m_executionUnits.emplace_back(std::make_unique<DeviceGroup>(m_ios, m_fileWatcher, child.first, child.second,
            m_constants));
    }
}

void Supervisor::start_waiting_for_termination_signals()
{
    m_signals.async_wait([=](auto& ec, int sig) {
        if (!ec) {
            this->on_termination_signal_received(sig);
        }
    });
}

void Supervisor::on_termination_signal_received(int sig)
{
    const char* sigStr = sig == SIGINT ? "SIGINT" : "SIGTERM";
    YOGI_LOG_INFO(sigStr << " received");

    m_ios.stop();
}

void Supervisor::start_execution_units()
{
    for (auto& unit : m_executionUnits) {
        unit->start();
    }
}
