#ifndef ECHOER_HH
#define ECHOER_HH

#include <yogi.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>


class Echoer
{
public:
    Echoer(const yogi::Configuration& config);

    void run();

private:
    typedef std::vector<char> byte_array;

    static constexpr unsigned kRawTerminalSignature = 0xFF;

    const yogi::Configuration&                      m_config;
    const yogi::Path                                m_location;
    const std::chrono::milliseconds                 m_serverTimeout;

    boost::asio::io_service                         m_ios;
    boost::asio::io_service::work                   m_work;
    boost::asio::signal_set                         m_signals;

    yogi::Scheduler                                 m_scheduler;
    yogi::Leaf                                      m_leaf;
    yogi::AutoConnectingTcpClient                   m_tcpClient;
    std::unique_ptr<yogi::TcpServer>                m_tcpServer;
    std::unique_ptr<yogi::TcpConnection>            m_serverConnection;

    yogi::RawMasterTerminal                         m_masterTerminal;
    yogi::MessageObserver<yogi::RawMasterTerminal>  m_masterObserver;

    yogi::RawServiceTerminal                        m_serviceTerminal;
    yogi::MessageObserver<yogi::RawServiceTerminal> m_serviceObserver;

    static yogi::Path resolve_hostname(const yogi::Path& path);

    void create_tcp_server();
    void start_accepting_connection();
    void on_connection_accepted();
    void on_server_connection_died(const yogi::Failure& err);
    void start_waiting_for_termination_signals();
    void on_termination_signal_received(int sig);
    void on_slave_message_received(const byte_array& data);
    void on_request_received(yogi::RawServiceTerminal::Request&& request);
};

#endif // ECHOER_HH
