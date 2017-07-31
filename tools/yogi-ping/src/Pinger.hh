#ifndef PINGER_HH
#define PINGER_HH

#include <yogi.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <deque>


class Pinger
{
public:
    Pinger(std::chrono::microseconds interval, std::size_t count, std::size_t payloadSize,
        bool useServiceTerminals, const std::string& host, unsigned port,
        const yogi::Path& echoerLocation);

    void run();

private:
    typedef std::vector<char>                     byte_array;
    typedef std::unique_ptr<yogi::TcpConnection>  connection_ptr;
    typedef std::chrono::high_resolution_clock    clock_type;
    typedef clock_type::time_point                time_point;
    typedef clock_type::duration                  duration_type;

    static constexpr unsigned kRawTerminalSignature  = 0xFF;
    static constexpr unsigned kConnectionTimeoutInMs = 3000;
    static constexpr unsigned kTerminalTimeoutInMs   = 1000;
    static constexpr unsigned kPingTimeoutInSeconds  = 3;

    const std::chrono::microseconds    m_interval;
    const std::size_t                  m_count;
    const bool                         m_useServiceTerminals;
    const std::string                  m_host;
    const unsigned                     m_port;
    const yogi::Path                   m_echoerLocation;
    const std::chrono::milliseconds    m_timeout;

    boost::asio::io_service            m_ios;
    boost::asio::io_service::work      m_work;
    boost::asio::signal_set            m_signals;
    boost::asio::deadline_timer        m_timer;

    yogi::Scheduler                    m_scheduler;
    yogi::Leaf                         m_leaf;
    yogi::TcpClient                    m_tcpClient;
    connection_ptr                     m_connection;

    yogi::RawSlaveTerminal             m_slaveTerminal;
    yogi::RawClientTerminal            m_clientTerminal;

    byte_array                         m_pingPayload;
    time_point                         m_pingSentTime;
    std::deque<duration_type>          m_times;
    std::size_t                        m_timeouts;
    yogi::RawClientTerminal::Operation m_clientOperation;

    void start_connect();
    void on_connect_completed(const yogi::Result& result, connection_ptr connection);
    void start_await_terminal_ready(unsigned iterations);
    void on_terminal_ready();
    void send_ping();
    void start_ping_timeout();
    void on_ping_timed_out();
    void start_observing_ms_pongs();
    void on_pong_received(const byte_array& pongPayload, const time_point& recvTime);
    void fill_payload();
    void send_next_ping_after_interval();
    void start_waiting_for_termination_signals();
    void on_termination_signal_received(int sig);
    void print_summary();
};

#endif // PINGER_HH
