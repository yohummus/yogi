#ifndef MANAGER_HH
#define MANAGER_HH

#include <yogi.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

#include "accounts/AccountManager.hh"
#include "sessions/SessionManager.hh"


class Manager
{
public:
    Manager();

    void run();

private:
    boost::asio::io_service  m_ios;
    boost::asio::signal_set  m_signals;
    accounts::AccountManager m_accountManager;
    sessions::SessionManager m_sessionManager;

    void start_waiting_for_termination_signals();
    void on_termination_signal_received(int sig);
};

#endif // MANAGER_HH
