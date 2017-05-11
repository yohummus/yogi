#include "DeviceGroup.hh"

#include <boost/algorithm/string.hpp>

#include <istream>
#include <algorithm>
using namespace std::string_literals;


DeviceGroup::DeviceGroup(boost::asio::io_service& ios, std::string name,
    const yogi::ConfigurationChild& configChild, const template_string_vector& constants)
: m_ios(ios)
, m_discoveryTimer(ios)
, m_name(name)
, m_configChild(configChild)
, m_constants(constants)
, m_phase(STARTUP_PHASE)
{
    read_configuration();
}

const std::string& DeviceGroup::name() const
{
    return m_name;
}

void DeviceGroup::start()
{
    run_command(m_startupCommand, {}, [=](auto exitStatus, auto& out, auto& err) {
        this->on_startup_command_finished(exitStatus, out, err);
    });
}

void DeviceGroup::read_configuration()
{
    m_enabled = m_configChild.get<bool>("enabled");
    
    if (m_enabled) {
        YOGI_LOG_DEBUG("Found enabled device group " << m_name);
    }
    else {
        YOGI_LOG_DEBUG("Found disabled device group " << m_name);
    }

    m_discoveryInterval = extract_duration("discovery-interval");
    m_commandTimeout = extract_duration("command-timeout");
    m_logfile = extract_string("logfile");
    extract_files_monitored_for_changes();

    if (m_enabled) {
        if (m_discoveryInterval == std::chrono::milliseconds::max()) {
            YOGI_LOG_INFO("Device group " << m_name << " enabled without a discovery interval");
        }
        else {
            YOGI_LOG_INFO("Device group " << m_name << " enabled with a discovery interval of "
                << m_discoveryInterval.count() << " ms");
        }
        
        for (auto& file : m_filesMonitoredForChanges) {
            YOGI_LOG_INFO("Device group " << m_name << " will be restarted on changes to " << file);
        }
    }

    m_startupCommand = extract_command("startup", m_commandTimeout);
    m_discoveryCommand = extract_command("discovery", m_commandTimeout);

    check_command_not_empty("discovery");
    check_command_not_empty("execution");
}

std::chrono::milliseconds DeviceGroup::extract_duration(const std::string& childName)
{
    auto val = m_configChild.get<float>(childName);
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

TemplateString DeviceGroup::extract_string(const std::string& childName)
{
    auto val = m_configChild.get<std::string>(childName, "");
    auto name = childName.rfind('.') == std::string::npos ? childName : childName.substr(childName.rfind('.') + 1);
    auto ts = TemplateString(name, val);
    ts.resolve(m_constants);
    return ts;
}

command_ptr DeviceGroup::extract_command(const std::string& childName, std::chrono::milliseconds timeout)
{
    auto cmdName = childName + " command for " + m_name;
    auto ts = extract_string("commands."s + childName);

    return std::make_unique<Command>(m_ios, cmdName, timeout, ts);
}

void DeviceGroup::check_command_not_empty(const std::string& cmdName)
{
    auto ts = extract_string("commands."s + cmdName);
    if (ts.value().empty()) {
        std::ostringstream oss;
        oss << "Command " << *this << ".commands." << cmdName << " is not set";
        throw std::runtime_error(oss.str());
    }
}

void DeviceGroup::extract_files_monitored_for_changes()
{
    for (auto child : m_configChild.get_child("restart-on-file-change")) {
        auto file = child.second.get_value<std::string>();
        auto ts = TemplateString(file);
        ts.resolve(m_constants);
        m_filesMonitoredForChanges.push_back(ts);
    }
}

void DeviceGroup::run_command(const command_ptr& cmd, const template_string_vector& variables,
    std::function<void (Command::exit_status_t, const std::string&, const std::string&)> completionHandler)
{
    if (cmd->empty()) {
        YOGI_LOG_TRACE("Skipping " << *cmd << " since the command is unset");
        completionHandler(Command::SUCCESS, {}, {});
        return;
    }
    
    cmd->async_run(variables, [=, cmd = cmd.get()](auto exitStatus, auto& out, auto& err) {
        if (exitStatus == Command::SUCCESS) {
            YOGI_LOG_DEBUG("Successfully executed " << *cmd);
            log_command_output(out, err, false);
        }
        else {
            YOGI_LOG_ERROR("Error while executing " << *cmd << ": " << exitStatus);
            log_command_output(out, err, true);
        }
        
        completionHandler(exitStatus, out, err);
});
}

void DeviceGroup::async_execute_discovery_phase_after_timeout()
{
    m_phase = IDLE_PHASE;
    if (m_discoveryInterval == std::chrono::milliseconds::max()) {
        YOGI_LOG_INFO("No discovery interval set for " << *this << " => no more devices will be discovered");
    }
    else {
        YOGI_LOG_TRACE("Sleeping before executing discovery phase for " << *this << "...");
        m_discoveryTimer.expires_from_now(boost::posix_time::milliseconds(m_discoveryInterval.count()));
        m_discoveryTimer.async_wait([=](auto& ec) {
            if (!ec) {
                this->execute_discovery_phase();
            }
        });
    }
}

void DeviceGroup::on_startup_command_finished(Command::exit_status_t exitStatus,
    const std::string& out, const std::string& err)
{
    execute_discovery_phase();
}

void DeviceGroup::execute_discovery_phase()
{
    m_phase = DISCOVERY_PHASE;
    run_command(m_discoveryCommand, {}, [=](auto exitStatus, auto& out, auto& err) {
        this->on_discovery_command_finished(exitStatus, out, err);
    });
}

void DeviceGroup::on_discovery_command_finished(Command::exit_status_t exitStatus,
    const std::string& out, const std::string& err)
{
    if (exitStatus != Command::SUCCESS) {
        async_execute_discovery_phase_after_timeout();
    }
    else {
        auto devices = parse_devices(out);
        YOGI_LOG_TRACE("Discovered " << devices.size() << " devices for " << *this << ":");
        for (auto& device : devices) {
            YOGI_LOG_TRACE(device);
        }

        for (auto& device : devices) {
            if (m_usedDevices.emplace(std::make_pair(device, command_ptr())).second) {
                execute_validation(device);
            }
        }

        async_execute_discovery_phase_after_timeout();
    }
}

void DeviceGroup::execute_validation(const std::string& device)
{
    command_ptr& cmd = m_usedDevices[device];
    cmd = extract_command("validation", m_commandTimeout);

    if (cmd->empty()) {
        YOGI_LOG_TRACE("Skipping " << *cmd << " since the command is unset");
        execute_activation(device);
    }
    else {
        cmd->async_run(make_device_variables(device), [=](auto exitStatus, auto& out, auto& err) {
            this->on_validation_command_finished(device, exitStatus, out, err);
        });
    }
}

void DeviceGroup::on_validation_command_finished(const std::string& device, Command::exit_status_t exitStatus,
    const std::string& out, const std::string& err)
{
    if (exitStatus == Command::SUCCESS) {
        YOGI_LOG_DEBUG("Device " << device << " successfully validated for " << *this);
        this->log_command_output(out, err, false);
        execute_activation(device);
    }
    else {
        if (exitStatus == Command::FAILURE) {
            YOGI_LOG_TRACE("Device " << device << " validated unsuccessfully for " << *this);
            this->log_command_output(out, err, false);
        }
        else {
            command_ptr& cmd = m_usedDevices[device];
            YOGI_LOG_ERROR("Error while running " << *cmd << ": " << exitStatus);
            this->log_command_output(out, err, true);
        }

        m_usedDevices.erase(device);
    }
}

void DeviceGroup::execute_activation(const std::string& device)
{
    command_ptr& cmd = m_usedDevices[device];
    cmd = extract_command("activation", m_commandTimeout);

    run_command(cmd, make_device_variables(device), [=](auto exitStatus, auto& out, auto& err) {
        this->on_activation_command_finished(device, exitStatus, out, err);
    });
}

void DeviceGroup::on_activation_command_finished(const std::string& device, Command::exit_status_t exitStatus,
    const std::string& out, const std::string& err)
{
    if (exitStatus == Command::SUCCESS) {
        execute_execution(device);
    }
    else {
        m_usedDevices.erase(device);
    }
}

void DeviceGroup::execute_execution(const std::string& device)
{
    command_ptr& cmd = m_usedDevices[device];
    cmd = extract_command("execution", std::chrono::milliseconds::max());

    YOGI_LOG_INFO("Starting " << *cmd << " for device " << device);
    cmd->async_run(make_device_variables(device), [=](auto exitStatus, auto& out, auto& err) {
        this->on_execution_command_finished(device, exitStatus, out, err);
    });
}

void DeviceGroup::on_execution_command_finished(const std::string& device, Command::exit_status_t exitStatus,
    const std::string& out, const std::string& err)
{
    command_ptr& cmd = m_usedDevices[device];

    if (exitStatus == Command::SUCCESS) {
        YOGI_LOG_INFO("Process " << *cmd << " for device " << device << " terminated gracefully");
    }
    else {
        YOGI_LOG_ERROR("Process " << *cmd << " for device " << device << " died: " << exitStatus);
    }

    m_usedDevices.erase(device);
}

std::vector<std::string> DeviceGroup::parse_devices(const std::string& out) const
{
    // split string by whitespace
    std::istringstream buf(out);
    std::vector<std::string> devices{std::istream_iterator<std::string>(buf),
        std::istream_iterator<std::string>()};
    return devices;
}

void DeviceGroup::log_command_output(const std::string& out, const std::string& err, bool logAsError)
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

template_string_vector DeviceGroup::make_device_variables(const std::string& device)
{
    template_string_vector variables;
    variables.emplace_back("DEVICE", device);

    if (device.find('/')) {
        variables.emplace_back("DEVICE_NAME", device);
    }
    else {
        variables.emplace_back("DEVICE_NAME", device.substr(device.rfind('/') + 1));
    }

    return variables;
}

std::ostream& operator<< (std::ostream& os, const DeviceGroup& group)
{
    return os << group.name();
}
