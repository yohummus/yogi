#ifndef SUPERVISOR_HH
#define SUPERVISOR_HH

#include "TemplateString.hh"
#include "DeviceGroup.hh"

#include <yogi.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>


class Supervisor
{
public:
    Supervisor();

    void run();

private:
    boost::asio::io_service       m_ios;
    boost::asio::io_service::work m_work;
    boost::asio::signal_set       m_signals;

    template_string_vector        m_constants;
    device_group_ptr_vector       m_deviceGroups;

    void extract_constants();
    template_string_vector extract_raw_constants();
    void extract_device_groups();
    void start_waiting_for_termination_signals();
    void on_termination_signal_received(int sig);
    void start_device_groups();
};

#endif // SUPERVISOR_HH
