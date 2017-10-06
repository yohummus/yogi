#include "TcpClient.hpp"
#include "../../yogi_core.h"

#include <boost/log/trivial.hpp>


namespace yogi {
namespace connections {
namespace tcp {

void TcpClient::start_async_resolve(std::string host, unsigned short port)
{
    BOOST_LOG_TRIVIAL(debug) << "Resolving " << host << ":" << std::dec << port;

    boost::asio::ip::tcp::resolver::query query{host, std::to_string(port)};
    m_resolver.async_resolve(query,
        [=](const boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator epIt) {
                on_resolve_completed(ec, epIt);
        }
    );
}

void TcpClient::on_resolve_completed(const boost::system::error_code & ec,
    boost::asio::ip::tcp::resolver::iterator epIt)
{
    // success?
    if (!ec) {
        auto lock = super::make_lock_guard();
        if (m_canceled) {
            m_connectOp.fire<YOGI_ERR_CANCELED>(tcp_connection_ptr{});
        }
        else {
            start_async_connect(epIt);
        }
    }
    // canceled?
    else if (ec == boost::asio::error::operation_aborted) {
        BOOST_LOG_TRIVIAL(debug) << "Async resolve operation canceled";
        m_connectOp.fire<YOGI_ERR_CANCELED>(tcp_connection_ptr{});
    }
    // error
    else {
        BOOST_LOG_TRIVIAL(error) << "Async resolve operation failed: "
            << ec.message();
        m_connectOp.fire<YOGI_ERR_RESOLVE_FAILED>(tcp_connection_ptr{});
    }
}

void TcpClient::start_async_connect(
    boost::asio::ip::tcp::resolver::iterator epIt)
{
    BOOST_LOG_TRIVIAL(debug) << "Connecting to " << epIt->endpoint().address()
        << ":" << std::dec << epIt->endpoint().port();

    m_socket.async_connect(*epIt,
        [=](const boost::system::error_code& ec) {
            on_connect_completed(ec, epIt);
        }
    );
}

void TcpClient::on_connect_completed(const boost::system::error_code & ec,
    boost::asio::ip::tcp::resolver::iterator epIt)
{
    // success?
    if (!ec) {
        auto lock = super::make_lock_guard();
        if (m_canceled) {
            m_connectOp.fire<YOGI_ERR_CANCELED>(tcp_connection_ptr{});
        }
        else {
            m_shakingHands = true;
            start_async_shake_hands(std::move(m_socket), m_handshakeTimeout);
        }
    }
    // canceled?
    else if (ec == boost::asio::error::operation_aborted) {
        BOOST_LOG_TRIVIAL(debug) << "Async connect operation canceled";
        m_connectOp.fire<YOGI_ERR_CANCELED>(tcp_connection_ptr{});
    }
    // error
    else {
        BOOST_LOG_TRIVIAL(error) << "Async connect operation failed: "
            << ec.message();

        if (ec == boost::asio::error::connection_refused) {
            m_connectOp.fire<YOGI_ERR_CONNECTION_REFUSED>(
                tcp_connection_ptr{});
        }
        else if (ec == boost::asio::error::host_unreachable) {
            m_connectOp.fire<YOGI_ERR_HOST_UNREACHABLE>(tcp_connection_ptr{});
        }
        else if (ec == boost::asio::error::network_down) {
            m_connectOp.fire<YOGI_ERR_NETWORK_DOWN>(tcp_connection_ptr{});
        }
        else {
            m_connectOp.fire<YOGI_ERR_CONNECT_FAILED>(tcp_connection_ptr{});
        }
    }
}

void TcpClient::on_shake_hands_completed(const api::Exception & e,
    tcp_connection_ptr&& conn)
{
    if (e.error_code() == YOGI_OK) {
        BOOST_LOG_TRIVIAL(debug) << "TCP connection to " << conn->description()
            << " successfully established";
    }

    m_connectOp.fire(e, std::move(conn));
}

TcpClient::TcpClient(interfaces::IScheduler & scheduler,
    identification_buffer identification)
    : super      {scheduler, identification}
    , m_scheduler{scheduler.make_ptr<interfaces::IScheduler>()}
    , m_resolver {scheduler.io_service()}
    , m_socket   {scheduler.io_service()}
{
}

TcpClient::~TcpClient()
{
    cancel_connect();
    m_connectOp.await_idle();

    // async operations may still be holding the lock for a short amount of time
    auto lock = super::make_lock_guard();
}

void TcpClient::async_connect(std::string host, unsigned short port,
    connect_handler_fn handlerFn, std::chrono::milliseconds handShakeTimeout)
{
    auto lock = super::make_lock_guard();

    m_connectOp.arm(handlerFn);

    m_socket = boost::asio::ip::tcp::socket{m_scheduler->io_service()};
    m_handshakeTimeout = handShakeTimeout;
    m_shakingHands = false;
    m_canceled = false;
    start_async_resolve(host, port);
}

void TcpClient::cancel_connect()
{
    auto lock = super::make_lock_guard();

    m_canceled = true;

    m_resolver.cancel();
    if (m_shakingHands) {
        super::cancel_shake_hands();
    }
    else {
        boost::system::error_code ec;
        m_socket.cancel(ec);
    }
}

} // namespace tcp
} // namespace connections
} // namespace yogi
