#include "DeviceGroup.hh"

#include <boost/filesystem.hpp>
using namespace std::string_literals;


DeviceGroup::DeviceGroup(boost::asio::io_service& ios, FileWatcher& fileWatcher, std::string name,
    const yogi::ConfigurationChild& configChild, const template_string_vector& constants)
: ExecutionUnit(ios, fileWatcher, name, configChild, "defaults.device-groups", constants)
{
    read_configuration();
    set_variable("DEVICE_GROUP", name);
}

DeviceGroup::~DeviceGroup()
{
    kill_active_commands();
    kill_active_timers();
}

void DeviceGroup::on_startup_command_finished_successfully()
{
    start_watching_devices();
}

void DeviceGroup::on_watched_file_changed()
{
}

void DeviceGroup::read_configuration()
{
    m_devices = extract_string("devices");
    if (m_devices.value().empty()) {
        throw std::runtime_error("Option 'devices' for "s + name() + " is not set");
    }

    namespace fs = boost::filesystem;
    auto dir = fs::path(m_devices.value()).parent_path();
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        throw std::runtime_error("Device directory "s + dir.native() + " for " + name() + " does not exist");
    }

    m_validationTimeout = extract_duration("validation-timeout");
    m_preExecutionTimeout = extract_duration("pre-execution-timeout");
}

void DeviceGroup::start_watching_devices()
{
    file_watcher().watch(m_devices.value(), [=](auto& filename, auto eventType) {
        this->on_device_file_changed(filename, eventType);
    }, true);
}

void DeviceGroup::on_device_file_changed(const std::string& filename, FileWatcher::event_type_t eventType)
{
    if (eventType == FileWatcher::FILE_CREATED) {
        YOGI_LOG_INFO("Found new device " << filename << " for " << name());

        auto vars = make_variables_for_device(filename);
        run_validation_command(filename, vars);
    }
    else if (eventType == FileWatcher::FILE_DELETED) {
        YOGI_LOG_INFO("Device " << filename << " for " << name() << " has been removed");
    }
}

void DeviceGroup::run_validation_command(const std::string& device, template_string_vector_ptr vars)
{
    YOGI_LOG_INFO("Starting validation command on " << device << " for " << name() << "...");
    
    auto cmd = extract_command("validation-command", m_validationTimeout);
    m_activeCommands.insert(cmd);
    
    run_command(cmd, *vars, [=](auto exitStatus, auto& out, auto& err) {
        this->on_validation_command_finished(exitStatus, device, vars);
        m_activeCommands.erase(cmd);
    }, false);
}

void DeviceGroup::on_validation_command_finished(Command::exit_status_t exitStatus,
    const std::string& device, template_string_vector_ptr vars)
{
    if (exitStatus == Command::SUCCESS) {
        YOGI_LOG_INFO("Successfully validated " << device << " for " << name());
        run_pre_execution_command(device, vars);
    }
    else {
        YOGI_LOG_INFO("Validation of " << device << " for " << name() << " failed");
    }
}

void DeviceGroup::run_pre_execution_command(const std::string& device, template_string_vector_ptr vars)
{
    YOGI_LOG_INFO("Starting pre-execution command on " << device << " for " << name() << "...");
    
    auto cmd = extract_command("pre-execution-command", m_preExecutionTimeout);
    m_activeCommands.insert(cmd);
    
    run_command(cmd, *vars, [=](auto exitStatus, auto& out, auto& err) {
        if (exitStatus == Command::KILLED) {
            YOGI_LOG_TRACE("Pre-execution command on " << device << " for " << this->name() << " finished with status " << exitStatus);
        }
        else if (exitStatus == Command::SUCCESS) {
            this->on_pre_execution_command_succeeded(device, vars);
        }
        else {
            YOGI_LOG_ERROR("Pre-execution common on " << device << " for " << this->name() << " failed with status " << exitStatus);
            this->start_restart_timer(device, vars);
        }
        m_activeCommands.erase(cmd);
    });
}

void DeviceGroup::on_pre_execution_command_succeeded(const std::string& device, template_string_vector_ptr vars)
{
    run_execution_command(device, vars);
}

void DeviceGroup::run_execution_command(const std::string& device, template_string_vector_ptr vars)
{
    YOGI_LOG_INFO("Starting execution command on " << device << " for " << name() << "...");
    
    auto cmd = extract_command("execution-command", std::chrono::milliseconds::max());
    m_activeCommands.insert(cmd);
    
    auto log = logfile();
    log.resolve(*vars);

    cmd->async_run(*vars, log, [=](auto exitStatus, auto& out, auto& err) {
        this->on_execution_command_finished(exitStatus, device, vars);
        m_activeCommands.erase(cmd);
    });
}

void DeviceGroup::on_execution_command_finished(Command::exit_status_t exitStatus,
    const std::string& device, template_string_vector_ptr vars)
{
    if (exitStatus == Command::KILLED) {
        YOGI_LOG_TRACE("Execution command on " << device << " for " << name() << " finished with status " << exitStatus);
    }
    else {
        std::ostringstream ss;
        ss << "Execution command on " << device << " for " << name() << " finished with status " << exitStatus;
        if (exitStatus == Command::SUCCESS) {
            YOGI_LOG_WARNING(ss.str());
        }
        else {
            YOGI_LOG_ERROR(ss.str());
        }

        start_restart_timer(device, vars);
    }
}

void DeviceGroup::start_restart_timer(const std::string& device, template_string_vector_ptr vars)
{
    auto timer = std::make_shared<boost::asio::deadline_timer>(io_service());
    start_timer(*timer, restart_delay(), [=, timer=timer] {
        run_pre_execution_command(device, vars);
    });
}

template_string_vector_ptr DeviceGroup::make_variables_for_device(const std::string& device)
{
    auto vars = std::make_shared<template_string_vector>(variables());
    vars->push_back(TemplateString("DEVICE", device));
    vars->push_back(TemplateString("DEVICE_NAME", boost::filesystem::path(device).filename().native()));
    return vars;
}

void DeviceGroup::kill_active_commands()
{
    for (auto& cmd : m_activeCommands) {
        cmd->kill();
    }

    m_activeCommands.clear();
}

void DeviceGroup::kill_active_timers()
{
    for (auto& timer : m_activeTimers) {
        timer->cancel();
    }

    m_activeTimers.clear();
}
