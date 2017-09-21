#include "Manager.hh"
#include "storage/FilesystemStorageProvider.hh"


using namespace std::string_literals;


Manager::Manager()
: m_signals(m_ios, SIGINT, SIGTERM)
, m_accountManager(m_ios)
, m_sessionManager(m_accountManager)
{
}

void Manager::run()
{
    start_waiting_for_termination_signals();
    m_ios.run();
}

void Manager::start_waiting_for_termination_signals()
{
    m_signals.async_wait([=](auto& ec, int sig) {
        if (!ec) {
            this->on_termination_signal_received(sig);
        }
    });
}

void Manager::on_termination_signal_received(int sig)
{
    const char* sigStr = sig == SIGINT ? "SIGINT" : "SIGTERM";
    YOGI_LOG_INFO(sigStr << " received.");

    m_ios.stop();
}
