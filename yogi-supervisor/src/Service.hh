#ifndef SERVICE_HH
#define SERVICE_HH

#include "ExecutionUnit.hh"
#include "FileWatcher.hh"

#include <yogi.hpp>


class Service : public ExecutionUnit
{
public:
    Service(boost::asio::io_service& ios, FileWatcher& fileWatcher, std::string name,
        const yogi::ConfigurationChild& configChild, const template_string_vector& constants);
    virtual ~Service();

private:
    std::chrono::milliseconds   m_restartDelay;
    command_ptr                 m_executionCommand;
    boost::asio::deadline_timer m_restartTimer;

protected:
    virtual void on_startup_command_finished_successfully() override;
    virtual void on_watched_file_changed() override;

private:
    void read_configuration();
    void run_execution_command();
    void on_execution_command_finished(Command::exit_status_t exitStatus);
};

#endif // SERVICE_HH
