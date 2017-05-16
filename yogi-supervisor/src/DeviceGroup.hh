#ifndef DEVICE_GROUP
#define DEVICE_GROUP

#include "ExecutionUnit.hh"
#include "FileWatcher.hh"

#include <yogi.hpp>
#include <unordered_set>


class DeviceGroup : public ExecutionUnit
{
    typedef std::shared_ptr<boost::asio::deadline_timer> timer_ptr;

public:
    DeviceGroup(boost::asio::io_service& ios, FileWatcher& fileWatcher, std::string name,
        const yogi::ConfigurationChild& configChild, const template_string_vector& constants);
    virtual ~DeviceGroup();

private:
    TemplateString                  m_devices;
    std::chrono::milliseconds       m_validationTimeout;
    std::chrono::milliseconds       m_preExecutionTimeout;
    std::unordered_set<command_ptr> m_activeCommands;
    std::unordered_set<timer_ptr>   m_activeTimers;

protected:
    virtual void on_startup_command_finished_successfully() override;
    virtual void on_watched_file_changed() override;

private:
    void read_configuration();
    void start_watching_devices();
    void on_device_file_changed(const std::string& filename, FileWatcher::event_type_t eventType);
    void run_validation_command(const std::string& device, template_string_vector_ptr vars);
    void on_validation_command_finished(Command::exit_status_t exitStatus,
        const std::string& device, template_string_vector_ptr vars);
    void run_pre_execution_command(const std::string& device, template_string_vector_ptr vars);
    void on_pre_execution_command_succeeded(const std::string& device, template_string_vector_ptr vars);
    void run_execution_command(const std::string& device, template_string_vector_ptr vars);
    void on_execution_command_finished(Command::exit_status_t exitStatus,
        const std::string& device, template_string_vector_ptr vars);
    void start_restart_timer(const std::string& device, template_string_vector_ptr vars);
    template_string_vector_ptr make_variables_for_device(const std::string& device);
    void kill_active_commands();
    void kill_active_timers();
};

#endif // DEVICE_GROUP
