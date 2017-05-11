#ifndef DEVICE_GROUP_HH
#define DEVICE_GROUP_HH

#include "TemplateString.hh"
#include "Command.hh"

#include <yogi.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <set>
#include <chrono>
#include <memory>


class DeviceGroup
{
public:
    DeviceGroup(boost::asio::io_service& ios, std::string name, const yogi::ConfigurationChild& configChild,
        const template_string_vector& constants);

    const std::string& name() const;
    void start();

private:
    enum phase_t {
        STARTUP_PHASE,
        IDLE_PHASE,
        DISCOVERY_PHASE
    };

    boost::asio::io_service&           m_ios;
    boost::asio::deadline_timer        m_discoveryTimer;
    std::string                        m_name;
    yogi::ConfigurationChild           m_configChild;
    template_string_vector             m_constants;
    bool                               m_enabled;
    std::chrono::milliseconds          m_discoveryInterval;
    std::chrono::milliseconds          m_commandTimeout;
    TemplateString                     m_logfile;
    phase_t                            m_phase;
    command_ptr                        m_startupCommand;
    command_ptr                        m_discoveryCommand;
    template_string_vector             m_filesMonitoredForChanges;
    std::map<std::string, command_ptr> m_usedDevices;

private:
    void read_configuration();
    std::chrono::milliseconds extract_duration(const std::string& childName);
    TemplateString extract_string(const std::string& childName);
    command_ptr extract_command(const std::string& childName, std::chrono::milliseconds timeout);
    void check_command_not_empty(const std::string& cmdName);
    void extract_files_monitored_for_changes();
    void run_command(const command_ptr& cmd, const template_string_vector& variables,
        std::function<void (Command::exit_status_t, const std::string&, const std::string&)> completionHandler);
    void async_execute_discovery_phase_after_timeout();
    void on_startup_command_finished(Command::exit_status_t exitStatus,
        const std::string& out, const std::string& err);
    void execute_discovery_phase();
    void on_discovery_command_finished(Command::exit_status_t exitStatus,
        const std::string& out, const std::string& err);
    void execute_validation(const std::string& device);
    void on_validation_command_finished(const std::string& device, Command::exit_status_t exitStatus,
        const std::string& out, const std::string& err);
    void execute_activation(const std::string& device);
    void on_activation_command_finished(const std::string& device, Command::exit_status_t exitStatus,
        const std::string& out, const std::string& err);
    void execute_execution(const std::string& device);
    void on_execution_command_finished(const std::string& device, Command::exit_status_t exitStatus,
        const std::string& out, const std::string& err);
    std::vector<std::string> parse_devices(const std::string& out) const;
    static void log_command_output(const std::string& out, const std::string& err, bool logAsError);
    static template_string_vector make_device_variables(const std::string& device);
};

typedef std::unique_ptr<DeviceGroup> device_group_ptr;
typedef std::vector<device_group_ptr> device_group_ptr_vector;

std::ostream& operator<< (std::ostream& os, const DeviceGroup& group);

#endif // DEVICE_GROUP_HH
