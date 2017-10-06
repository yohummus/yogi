#include "TcpServer.hpp"
#include "../../yogi_core.h"

#include <boost/log/trivial.hpp>


namespace yogi {
namespace connections {
namespace tcp {

boost::asio::ip::tcp::endpoint TcpServer::make_endpoint(std::string address,
    unsigned short port)
{
    if (port == 0) {
        throw api::ExceptionT<YOGI_ERR_INVALID_PORT_NUMBER>{};
    }

    boost::system::error_code ec;
    auto addr = boost::asio::ip::address::from_string(address, ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Could not convert '" << address
            << "' to an IP address: " << ec.message();
        throw api::ExceptionT<YOGI_ERR_INVALID_IP_ADDRESS>{};
    }

    return boost::asio::ip::tcp::endpoint{addr, port};
}

void TcpServer::open_acceptor()
{
    boost::system::error_code ec;
    m_acceptor.open(m_endpoint.protocol(), ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Could not open acceptor socket: "
            << ec.message();
        throw api::ExceptionT<YOGI_ERR_CANNOT_OPEN_SOCKET>{};
    }

    boost::asio::socket_base::reuse_address option(true);
    m_acceptor.set_option(option);
}

void TcpServer::bind_acceptor()
{
    boost::system::error_code ec;
    m_acceptor.bind(m_endpoint, ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Could not bind acceptor socket: "
            << ec.message();

        if (ec == boost::asio::error::address_in_use) {
            throw api::ExceptionT<YOGI_ERR_ADDRESS_IN_USE>{};
        }
        else {
            throw api::ExceptionT<YOGI_ERR_CANNOT_BIND_SOCKET>{};
        }
    }
}

void TcpServer::listen_on_acceptor()
{
    boost::system::error_code ec;
    m_acceptor.listen(YOGI_TCP_ACCEPTOR_BACKLOG, ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Could not listen on acceptor socket: "
            << ec.message();
        throw api::ExceptionT<YOGI_ERR_CANNOT_LISTEN_ON_SOCKET>{};
    }
}

void TcpServer::start_async_accept()
{
    BOOST_LOG_TRIVIAL(debug) << "Starting async accept operation...";

    m_acceptor.async_accept(m_socket,
        [=](const boost::system::error_code& ec) {
            on_accept_completed(ec);
        }
    );
}

void TcpServer::on_accept_completed(const boost::system::error_code& ec)
{
    // success?
    if (!ec) {
        BOOST_LOG_TRIVIAL(debug) << "Incoming connection request from "
            << m_socket.remote_endpoint().address() << ":"
            << std::dec << m_socket.remote_endpoint().port();

        auto lock = super::make_lock_guard();
        if (m_canceled) {
            m_acceptOp.fire<YOGI_ERR_CANCELED>(tcp_connection_ptr{});
        }
        else {
            start_async_shake_hands(std::move(m_socket), m_handshakeTimeout);
        }
    }
    // canceled?
    else if (ec == boost::asio::error::operation_aborted) {
        BOOST_LOG_TRIVIAL(debug) << "Async accept operation canceled";
        m_acceptOp.fire<YOGI_ERR_CANCELED>(tcp_connection_ptr{});
    }
    // error
    else {
        BOOST_LOG_TRIVIAL(error) << "Async accept operation failed: "
            << ec.message();
        m_acceptOp.fire<YOGI_ERR_ACCEPT_FAILED>(tcp_connection_ptr{});
    }
}

void TcpServer::on_shake_hands_completed(const api::Exception& e,
    tcp_connection_ptr&& conn)
{
    if (e.error_code() == YOGI_OK) {
        BOOST_LOG_TRIVIAL(debug) << "TCP connection to " << conn->description()
            << " successfully established";
    }

    m_acceptOp.fire(e, std::move(conn));
}

TcpServer::TcpServer(interfaces::IScheduler& scheduler, std::string address,
    unsigned short port, identification_buffer identification)
    : super      {scheduler, identification}
    , m_scheduler{scheduler.make_ptr<interfaces::IScheduler>()}
    , m_endpoint {make_endpoint(address, port)}
    , m_acceptor {scheduler.io_service()}
    , m_socket   {scheduler.io_service()}
{
    open_acceptor();
    bind_acceptor();
    listen_on_acceptor();

    BOOST_LOG_TRIVIAL(info) << "Successfully created TcpServer on "
        << m_endpoint.address() << ":" << std::dec << m_endpoint.port();
}

TcpServer::~TcpServer()
{
    BOOST_LOG_TRIVIAL(info) << "Destroyed TcpServer on "
        << m_endpoint.address() << ":" << std::dec << m_endpoint.port();

    cancel_accept();
    m_acceptOp.await_idle();

    // async operations may still be holding the lock for a short amount of time
    auto lock = super::make_lock_guard();
}

void TcpServer::async_accept(accept_handler_fn handlerFn,
    std::chrono::milliseconds handShakeTimeout)
{
    auto lock = super::make_lock_guard();

    m_acceptOp.arm(handlerFn);

    m_socket = boost::asio::ip::tcp::socket{m_scheduler->io_service()};
    m_handshakeTimeout = handShakeTimeout;
    m_canceled = false;
    start_async_accept();
}

void TcpServer::cancel_accept()
{
    auto lock = super::make_lock_guard();

    m_canceled = true;

    boost::system::error_code ec;
    m_acceptor.cancel(ec);
    super::cancel_shake_hands();
}

} // namespace tcp
} // namespace connections
} // namespace yogi
