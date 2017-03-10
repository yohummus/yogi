#ifndef CHIRP_CONNECTIONS_TCP_TCPCONNECTIONFACTORY_HPP
#define CHIRP_CONNECTIONS_TCP_TCPCONNECTIONFACTORY_HPP

#include "../../config.h"
#include "../../interfaces/IScheduler.hpp"
#include "../../api/ExceptionT.hpp"
#include "TcpConnection.hpp"

#include <boost/asio/ip/tcp.hpp>

#include <vector>
#include <chrono>
#include <mutex>
#include <memory>


namespace chirp {
namespace connections {
namespace tcp {

/***************************************************************************//**
 * Base class for classes which create TcpConnections
 ******************************************************************************/
class TcpConnectionFactory : public interfaces::IPublicObject
{
public:
    typedef boost::asio::const_buffers_1 identification_buffer;

private:
    static std::vector<char> ms_magicPrefix;
    static std::vector<char> ms_versionInfo;

    const interfaces::scheduler_ptr m_scheduler;
    const std::vector<char>         m_identification;

    std::recursive_mutex            m_mutex;
    std::unique_ptr<api::Exception> m_firstResult;
    std::string                     m_remoteVersion;
    std::vector<char>               m_buffer;
    boost::asio::ip::tcp::socket    m_socket;
    boost::asio::deadline_timer     m_timer;
    bool                            m_canceled;

private:
    static std::vector<char> make_magic_prefix();
    static std::vector<char> make_version_info();
    static std::vector<char> make_identification(identification_buffer buffer);

private:
    bool versions_are_compatible();
    void completion_handler(const boost::system::error_code& ec,
        const char* opName, void (TcpConnectionFactory::*fn)());
    tcp_connection_ptr make_connection();
    void cancel_socket();

    template <int TErrorCode>
    void handle_protocol_error();

    void start_async_send_header();
    void on_send_header_succeeded();
    void start_async_receive_magic_prefix();
    void on_receive_magic_prefix_succeeded();
    void start_async_receive_version_info();
    void on_receive_version_info_succeeded();
    void start_async_receive_identification_size();
    void on_receive_identification_size_succeeded();
    void start_async_receive_identification_data();
    void on_receive_identification_data_succeeded();
    void on_timeout(const boost::system::error_code& ec);

protected:
    TcpConnectionFactory(interfaces::IScheduler& scheduler,
        identification_buffer identification);

    std::unique_lock<std::recursive_mutex> make_lock_guard();

    void start_async_shake_hands(boost::asio::ip::tcp::socket&& socket,
        std::chrono::milliseconds timeout);
    virtual void on_shake_hands_completed(const api::Exception& e,
        tcp_connection_ptr&& conn) =0;
    void cancel_shake_hands();
};

} // namespace tcp
} // namespace connections
} // namespace chirp

#endif // CHIRP_CONNECTIONS_TCP_TCPCONNECTIONFACTORY_HPP
