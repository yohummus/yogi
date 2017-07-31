#include "Pinger.hh"

#include <algorithm>
#include <numeric>

using namespace std::string_literals;


Pinger::Pinger(std::chrono::microseconds interval, std::size_t count, std::size_t payloadSize,
    bool useServiceTerminals, const std::string& host, unsigned port,
    const yogi::Path& echoerLocation)
: m_interval(interval)
, m_count(count)
, m_useServiceTerminals(useServiceTerminals)
, m_host(host)
, m_port(port)
, m_echoerLocation(echoerLocation)
, m_timeout(unsigned(kConnectionTimeoutInMs))
, m_work(m_ios)
, m_signals(m_ios, SIGINT, SIGTERM)
, m_timer(m_ios)
, m_leaf(m_scheduler)
, m_tcpClient(m_scheduler)
, m_slaveTerminal(m_leaf, m_echoerLocation, yogi::Signature(kRawTerminalSignature))
, m_clientTerminal(m_leaf, m_echoerLocation, yogi::Signature(kRawTerminalSignature))
, m_pingPayload(payloadSize)
, m_timeouts(0)
{
}

void Pinger::run()
{
    start_connect();
    start_waiting_for_termination_signals();
    m_ios.run();

    print_summary();
}

void Pinger::start_connect()
{
    m_tcpClient.async_connect(m_host, m_port, m_timeout,
        [=](auto& result, auto connection) {
            this->on_connect_completed(result, std::move(connection));
        }
    );
}

void Pinger::on_connect_completed(const yogi::Result& result,
    std::unique_ptr<yogi::TcpConnection> connection)
{
    if (result) {
        connection->assign(m_leaf, m_timeout);
        m_connection = std::move(connection);
        start_await_terminal_ready(unsigned(kTerminalTimeoutInMs));
    }
    else {
        std::cerr << "ERROR: Could not connect to " << m_host << ":" << m_port << ": "
                  << result << std::endl;
        m_ios.stop();
    }
}

void Pinger::start_await_terminal_ready(unsigned iterations)
{
    bool ready = false;
    if (m_useServiceTerminals) {
        ready = m_clientTerminal.get_subscription_state() == yogi::SUBSCRIBED;
    }
    else {
        ready = m_slaveTerminal.get_subscription_state() == yogi::SUBSCRIBED
                && m_slaveTerminal.get_binding_state() == yogi::ESTABLISHED;
    }

    if (ready) {
        on_terminal_ready();
    }
    else if (iterations > 0) {
        m_timer.expires_from_now(boost::posix_time::milliseconds(1));
        m_timer.async_wait([=](auto& ec) {
            this->start_await_terminal_ready(iterations - 1);
        });
    }
    else {
        std::cerr << "ERROR: Could not bind to target " << m_echoerLocation << std::endl;
        m_ios.stop();
    }
}

void Pinger::on_terminal_ready()
{
    if (!m_useServiceTerminals) {
        start_observing_ms_pongs();
    }

    send_ping();
}

void Pinger::send_ping()
{
    if (m_times.size() >= m_count) {
        m_ios.stop();
        return;
    }

    fill_payload();
    
    if (m_useServiceTerminals) {
        m_clientOperation = m_clientTerminal.async_request(m_pingPayload, [=](auto& res, auto&& response) {
            if (res && (response.flags() & ~yogi::gather_flags::FINISHED) == yogi::gather_flags::NO_FLAGS) {
                auto now = clock_type::now();
                auto pongPayload = response.data();
                this->m_ios.post([=] {
                    this->on_pong_received(pongPayload, now);
                });
            }

            return yogi::STOP;
        });
    }
    else {
        m_slaveTerminal.publish(m_pingPayload);
    }

    m_pingSentTime = clock_type::now();

    start_ping_timeout();
}

void Pinger::start_ping_timeout()
{
    m_timer.expires_from_now(boost::posix_time::seconds(kPingTimeoutInSeconds));
    m_timer.async_wait([=](auto& ec) {
        if (!ec) {
            this->on_ping_timed_out();
        }
    });
}

void Pinger::on_ping_timed_out()
{
    ++m_timeouts;
    
    printf("%d bytes from %s %s: timeout (> %d s)\n",
        static_cast<int>(m_pingPayload.size()),
        (m_useServiceTerminals ? "Service" : "Master"),
        m_echoerLocation.to_string().c_str(),
        kPingTimeoutInSeconds
    );

    if (m_clientOperation) {
        try {
            m_clientOperation.cancel();
        }
        catch (const yogi::Failure&) {
        }
    }

    send_ping();
}

void Pinger::start_observing_ms_pongs()
{
    m_slaveTerminal.async_receive_message([=](auto& res, auto&& pongPayload) {
        if (res) {
            auto now = clock_type::now();
            this->m_ios.post([=] {
                this->on_pong_received(pongPayload, now);
            });
        }

        if (res != yogi::errors::Canceled()) {
            this->start_observing_ms_pongs();
        }
    });
}

void Pinger::on_pong_received(const byte_array& pongPayload, const time_point& recvTime)
{
    if (pongPayload != m_pingPayload) {
        // we have interference from another processes pinging the same Terminal
        return;
    }

    m_clientOperation = {};

    auto dur = recvTime - m_pingSentTime;
    m_times.push_back(dur);

    auto time = std::chrono::duration_cast<std::chrono::duration<double>>(dur);
    printf("%d bytes from %s %s: time = %.03f ms\n",
        static_cast<int>(pongPayload.size()),
        (m_useServiceTerminals ? "Service" : "Master"),
        m_echoerLocation.to_string().c_str(),
        time.count() * 1000
    );

    send_next_ping_after_interval();
}

void Pinger::fill_payload()
{
    std::generate(m_pingPayload.begin(), m_pingPayload.end(), std::rand);
}

void Pinger::send_next_ping_after_interval()
{
    m_timer.expires_from_now(boost::posix_time::microseconds(m_interval.count()));
    m_timer.async_wait([=](auto& ec) {
        if (!ec) {
            this->send_ping();
        }
    });
}

void Pinger::start_waiting_for_termination_signals()
{
    m_signals.async_wait([=](auto& ec, int sig) {
        if (!ec) {
            this->on_termination_signal_received(sig);
        }
    });
}

void Pinger::on_termination_signal_received(int sig)
{
    m_ios.stop();
}

void Pinger::print_summary()
{
    if (!m_times.empty()) {
        auto begin = m_times.begin();
        auto end = m_times.end();

        auto count    = static_cast<int>(m_times.size() + m_timeouts);
        auto timeouts = static_cast<int>(m_timeouts);
        auto minDur   = *std::min_element(begin, end);
        auto maxDur   = *std::max_element(begin, end);
        auto avgDur   = std::accumulate(begin, end, duration_type(0)) / m_times.size();

        auto dur_to_ms = [](duration_type dur) {
            return std::chrono::duration_cast<std::chrono::duration<double>>(dur).count() * 1000;
        };

        printf("\n");
        printf("Number of pings sent:  %d\n", count);
        printf("Number of timeouts:    %d\n", timeouts);
        printf("Minimum response time: %.03f ms\n", dur_to_ms(minDur));
        printf("Average response time: %.03f ms\n", dur_to_ms(avgDur));
        printf("Maximum response time: %.03f ms\n", dur_to_ms(maxDur));
    }
}
