#ifndef YOGI_CONNECTIONS_TCP_TCPCLIENT_HPP
#define YOGI_CONNECTIONS_TCP_TCPCLIENT_HPP

#include "../../config.h"
#include "../../interfaces/IScheduler.hpp"
#include "../../base/AsyncOperation.hpp"
#include "TcpConnectionFactory.hpp"


namespace yogi {
namespace connections {
namespace tcp {

/***************************************************************************//**
 * Connects to a TCP/IP server
 ******************************************************************************/
class TcpClient : public TcpConnectionFactory
{
    typedef TcpConnectionFactory super;

public:
    typedef std::function<void (const api::Exception&,
        tcp_connection_ptr&&)> connect_handler_fn;

private:
    const interfaces::scheduler_ptr m_scheduler;

    boost::asio::ip::tcp::resolver           m_resolver;
    boost::asio::ip::tcp::socket             m_socket;
    base::AsyncOperation<connect_handler_fn> m_connectOp;
    std::chrono::milliseconds                m_handshakeTimeout;
    bool                                     m_shakingHands;
    bool                                     m_canceled;

private:
    void start_async_resolve(std::string host, unsigned short port);
    void on_resolve_completed(const boost::system::error_code& ec,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void start_async_connect(boost::asio::ip::tcp::resolver::iterator epIt);
    void on_connect_completed(const boost::system::error_code& ec,
        boost::asio::ip::tcp::resolver::iterator epIt);
    virtual void on_shake_hands_completed(const api::Exception& e,
        tcp_connection_ptr&& conn) override;

public:
    TcpClient(interfaces::IScheduler& scheduler,
        identification_buffer identification);
    virtual ~TcpClient();

    void async_connect(std::string host, unsigned short port,
        connect_handler_fn handlerFn,
        std::chrono::milliseconds handShakeTimeout);
    void cancel_connect();
};

} // namespace tcp
} // namespace connections
} // namespace yogi

#endif // YOGI_CONNECTIONS_TCP_TCPCLIENT_HPP
