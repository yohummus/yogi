#ifndef CHIRP_CONNECTIONS_TCP_TCPSERVER_HPP
#define CHIRP_CONNECTIONS_TCP_TCPSERVER_HPP

#include "../../config.h"
#include "../../interfaces/IScheduler.hpp"
#include "../../base/AsyncOperation.hpp"
#include "TcpConnectionFactory.hpp"

#include <chrono>
#include <functional>
#include <vector>
#include <unordered_map>


namespace chirp {
namespace connections {
namespace tcp {

/***************************************************************************//**
 * Listens for incoming TCP/IP connections
 ******************************************************************************/
class TcpServer : public TcpConnectionFactory
{
    typedef TcpConnectionFactory super;

public:
    typedef std::function<void (const api::Exception&,
        tcp_connection_ptr&&)> accept_handler_fn;

private:
    const interfaces::scheduler_ptr      m_scheduler;
    const boost::asio::ip::tcp::endpoint m_endpoint;

    boost::asio::ip::tcp::acceptor          m_acceptor;
    boost::asio::ip::tcp::socket            m_socket;
    base::AsyncOperation<accept_handler_fn> m_acceptOp;
    std::chrono::milliseconds               m_handshakeTimeout;
    bool                                    m_canceled;

private:
    static boost::asio::ip::tcp::endpoint make_endpoint(
        std::string address, unsigned short port);
    void open_acceptor();
    void bind_acceptor();
    void listen_on_acceptor();
    void start_async_accept();
    void on_accept_completed(const boost::system::error_code& ec);
    virtual void on_shake_hands_completed(const api::Exception& e,
        tcp_connection_ptr&& conn) override;

public:
    TcpServer(interfaces::IScheduler& scheduler, std::string address,
        unsigned short port, identification_buffer identification);
    virtual ~TcpServer();

    void async_accept(accept_handler_fn handlerFn,
        std::chrono::milliseconds handshakeTimeout);
    void cancel_accept();
};

} // namespace tcp
} // namespace connections
} // namespace chirp

#endif // CHIRP_CONNECTIONS_TCP_TCPSERVER_HPP
