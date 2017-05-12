#ifndef SERVICE_HH
#define SERVICE_HH

#include "ExecutionUnit.hh"

#include <yogi.hpp>


class Service : public ExecutionUnit
{
public:
    Service(boost::asio::io_service& ios, std::string name, const yogi::ConfigurationChild& configChild,
        const template_string_vector& constants);

private:
    std::chrono::milliseconds m_restartDelay;
    command_ptr               m_executionCommand;

protected:
    virtual void on_startup_command_finished_successfully() override;

private:
    void read_configuration();
};

#endif // SERVICE_HH
