#include "Echoer.hh"

#include <boost/algorithm/string/replace.hpp>

#include <unistd.h>
#include <limits.h>

using namespace std::string_literals;


Echoer::Echoer(const yogi::Configuration& config)
: m_config(config)
, m_location(resolve_hostname(config.location()))
, m_serverTimeout(config.get("yogi.server.timeout", std::chrono::milliseconds::max()))
, m_work(m_ios)
, m_signals(m_ios, SIGINT, SIGTERM)
, m_leaf(m_scheduler)
, m_tcpClient(m_leaf, config)
, m_masterTerminal(m_leaf, m_location, yogi::Signature(kRawTerminalSignature))
, m_masterObserver(m_masterTerminal)
, m_serviceTerminal(m_leaf, m_location, yogi::Signature(kRawTerminalSignature))
, m_serviceObserver(m_serviceTerminal)
{
    if (m_config.get_child_optional("yogi.connection") && m_config.get_child_optional("yogi.server")) {
        throw std::runtime_error("Multiple connection factories specified (yogi.connection"
            " and yogi.server). Please only specify one.");
    }

    m_masterObserver.add([=](auto& data) { this->on_slave_message_received(data); });
    m_serviceObserver.set([=](auto&& request) { this->on_request_received(std::move(request)); });

    create_tcp_server();
}

void Echoer::run()
{
    m_masterObserver.start();
    m_serviceObserver.start();

    if (!m_tcpClient.host().empty()) {
        m_tcpClient.start();
    }

    if (m_tcpServer) {
        start_accepting_connection();
    }

    start_waiting_for_termination_signals();
    m_ios.run();
}

yogi::Path Echoer::resolve_hostname(const yogi::Path& path)
{
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    auto s = path.to_string();
    boost::replace_all(s, "${HOSTNAME}", hostname);

    return s;
}

void Echoer::create_tcp_server()
{
    auto addr = m_config.get_optional<std::string>("yogi.server.address");
    if (addr) {
        if (*addr == "any") {
            *addr = "0.0.0.0";
        }

        auto port = m_config.get<unsigned>("yogi.server.port");
        auto ident = m_config.get_optional<std::string>("yogi.server.identification");

        m_tcpServer = std::make_unique<yogi::TcpServer>(m_scheduler, *addr, port, ident);
        YOGI_LOG_INFO("Listening for incoming connections on " << *addr << " port "
            << port << "...");
    }
}

void Echoer::start_accepting_connection()
{
    m_serverConnection.reset();

    m_tcpServer->async_accept(m_serverTimeout, [=](auto& res, auto connection) {
        if (res) {
            this->m_serverConnection = std::move(connection);
            m_ios.post([=] {
                this->on_connection_accepted();
            });
        }
        else if (res != yogi::errors::Canceled()) {
            YOGI_LOG_ERROR("Could not accept connection: " << res);
        }
    });
}

void Echoer::on_connection_accepted()
{
    YOGI_LOG_INFO("Accepted connection from " << m_serverConnection->description());

    m_serverConnection->async_await_death([=](auto& err) {
        m_ios.post([=] {
            this->on_server_connection_died(err);
        });
    });

    try {
        m_serverConnection->assign(m_leaf, m_serverTimeout);
    }
    catch (const yogi::Failure& e) {
        YOGI_LOG_ERROR("Could not assign connection to leaf: " << e.what());

        start_accepting_connection();
    }
}

void Echoer::on_server_connection_died(const yogi::Failure& err)
{
    if (err != yogi::errors::Canceled()) {
        YOGI_LOG_INFO("Connection from " << m_serverConnection->description() << " died: "
            << err.what())

        start_accepting_connection();
    }
}

void Echoer::start_waiting_for_termination_signals()
{
    m_signals.async_wait([=](auto& ec, int sig) {
        if (!ec) {
            this->on_termination_signal_received(sig);
        }
    });
}

void Echoer::on_termination_signal_received(int sig)
{
    const char* sigStr = sig == SIGINT ? "SIGINT" : "SIGTERM";
    YOGI_LOG_INFO(sigStr << " received");

    m_ios.stop();
}

void Echoer::on_slave_message_received(const byte_array& data)
{
    m_masterTerminal.try_publish(data);
}

void Echoer::on_request_received(yogi::RawServiceTerminal::Request&& request)
{
    request.try_respond(request.data());
}
