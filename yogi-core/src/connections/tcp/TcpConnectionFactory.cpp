#include "TcpConnectionFactory.hpp"
#include "../../yogi_core.h"

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>


namespace yogi {
namespace connections {
namespace tcp {

std::vector<char> TcpConnectionFactory::ms_magicPrefix = make_magic_prefix();
std::vector<char> TcpConnectionFactory::ms_versionInfo = make_version_info();

std::vector<char> TcpConnectionFactory::make_magic_prefix()
{
    return std::vector<char>{'C', 'H', 'I', 'R', 'P', ' '};
}

std::vector<char> TcpConnectionFactory::make_version_info()
{
    static std::string s{YOGI_VERSION};
    YOGI_ASSERT(s.size() < YOGI_VERSION_INFO_SIZE);
    std::vector<char> info(YOGI_VERSION_INFO_SIZE);
    std::copy(s.begin(), s.end(), info.begin());
    return info;
}

std::vector<char> TcpConnectionFactory::make_identification(
    identification_buffer buffer)
{
    const std::size_t bufSize = boost::asio::buffer_size(buffer);
    if (bufSize > YOGI_MAX_TCP_IDENTIFICATION_SIZE) {
        throw api::ExceptionT<YOGI_ERR_IDENTIFICATION_TOO_LARGE>{};
    }

    const std::size_t n = sizeof(std::uint32_t);
    std::vector<char> data(n + bufSize);
    std::uint32_t nBufSize = htonl(static_cast<unsigned long>(bufSize));
    std::copy(reinterpret_cast<char*>(&nBufSize),
        reinterpret_cast<char*>(&nBufSize) + n, data.begin());
    boost::asio::buffer_copy(boost::asio::buffer(data) + n, buffer);

    return data;
}

bool TcpConnectionFactory::versions_are_compatible()
{
    static std::string version{YOGI_VERSION};
    static std::string majorMinor = version.substr(0,
        version.find_last_of('.'));

    return m_remoteVersion.substr(0, m_remoteVersion.find_last_of('.'))
        == majorMinor;
}

void TcpConnectionFactory::completion_handler(
    const boost::system::error_code& ec, const char* opName,
    void (TcpConnectionFactory::*fn)())
{
    auto lock = make_lock_guard();

    // success?
    if (!m_canceled && !ec) {
        (this->*fn)();
    }
    // canceled?
    else if (m_canceled || ec == boost::asio::error::operation_aborted) {
        BOOST_LOG_TRIVIAL(debug) << "Async " << opName << " operation canceled";

        if (m_firstResult) {
            tcp_connection_ptr conn;
            // timeout
            if (m_firstResult->error_code() == YOGI_ERR_TIMEOUT) {
                on_shake_hands_completed(api::ExceptionT<YOGI_ERR_TIMEOUT>{},
                    std::move(conn));
                m_socket.close();
            }
            // all good
            else if (m_firstResult->error_code() == YOGI_ERR_CANCELED) {
                conn = std::make_shared<TcpConnection>(*m_scheduler,
                    std::move(m_socket), m_remoteVersion, m_buffer);
                on_shake_hands_completed(api::ExceptionT<YOGI_OK>{}, std::move(conn));
            }
            // some weird system error
            else {
                on_shake_hands_completed(*m_firstResult, std::move(conn));
                m_socket.close();
            }
        }
        else {
            m_firstResult = std::make_unique<api::ExceptionT<
                YOGI_ERR_CANCELED>>();
            m_timer.cancel();
        }
    }
    // error
    else {
        BOOST_LOG_TRIVIAL(error) << "Async " << opName << " operation failed: "
            << ec.message();

        auto lock = make_lock_guard();
        if (m_firstResult) {
            on_shake_hands_completed(api::ExceptionT<
                YOGI_ERR_SOCKET_BROKEN>{}, tcp_connection_ptr{});
        }
        else {
            m_firstResult = std::make_unique<api::ExceptionT<
                YOGI_ERR_SOCKET_BROKEN>>();
            m_timer.cancel();
        }
    }
}

tcp_connection_ptr TcpConnectionFactory::make_connection()
{
    return std::make_shared<TcpConnection>(*m_scheduler, std::move(m_socket),
        m_remoteVersion, m_buffer);
}

void TcpConnectionFactory::cancel_socket()
{
    boost::system::error_code ec;
    m_socket.cancel(ec);
    m_canceled = true;
}

template <int TErrorCode>
void TcpConnectionFactory::handle_protocol_error()
{
    auto lock = make_lock_guard();
    if (m_firstResult) {
        on_shake_hands_completed(api::ExceptionT<YOGI_ERR_TIMEOUT>{},
            tcp_connection_ptr{});
        m_socket.close();
    }
    else {
        m_firstResult = std::make_unique<api::ExceptionT<TErrorCode>>();
        m_timer.cancel();
    }
}

void TcpConnectionFactory::start_async_send_header()
{
    auto buffer = std::vector<boost::asio::const_buffer>{
        boost::asio::buffer(ms_magicPrefix),
        boost::asio::buffer(ms_versionInfo),
        boost::asio::buffer(m_identification)};

    boost::asio::async_write(m_socket, buffer,
        [=](const boost::system::error_code& ec, std::size_t) {
            completion_handler(ec, "write", &TcpConnectionFactory::
                on_send_header_succeeded);
        }
    );
}

void TcpConnectionFactory::on_send_header_succeeded()
{
    start_async_receive_magic_prefix();
}

void TcpConnectionFactory::start_async_receive_magic_prefix()
{
    m_buffer.resize(ms_magicPrefix.size());

    boost::asio::async_read(m_socket, boost::asio::buffer(m_buffer),
        [=](const boost::system::error_code& ec, std::size_t) {
            completion_handler(ec, "read", &TcpConnectionFactory::
                on_receive_magic_prefix_succeeded);
        }
    );
}

void TcpConnectionFactory::on_receive_magic_prefix_succeeded()
{
    if (m_buffer == ms_magicPrefix) {
        start_async_receive_version_info();
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Invalid magic prefix received: "
            << std::string(m_buffer.begin(), m_buffer.end());
        handle_protocol_error<YOGI_ERR_INVALID_MAGIC_PREFIX>();
    }
}

void TcpConnectionFactory::start_async_receive_version_info()
{
    m_buffer.resize(ms_versionInfo.size());

    boost::asio::async_read(m_socket, boost::asio::buffer(m_buffer),
        [=](const boost::system::error_code& ec, std::size_t) {
            completion_handler(ec, "read", &TcpConnectionFactory::
                on_receive_version_info_succeeded);
        }
    );
}

void TcpConnectionFactory::on_receive_version_info_succeeded()
{
    m_buffer.back() = '\0';
    m_remoteVersion = std::string(m_buffer.data());

    if (versions_are_compatible()) {
        start_async_receive_identification_size();
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Incompatible version received: "
            << m_remoteVersion;
        handle_protocol_error<YOGI_ERR_INCOMPATIBLE_VERSION>();
    }
}

void TcpConnectionFactory::start_async_receive_identification_size()
{
    m_buffer.resize(sizeof(std::uint32_t));

    boost::asio::async_read(m_socket, boost::asio::buffer(m_buffer),
        [=](const boost::system::error_code& ec, std::size_t) {
            completion_handler(ec, "read", &TcpConnectionFactory::
                on_receive_identification_size_succeeded);
        }
    );
}

void TcpConnectionFactory::on_receive_identification_size_succeeded()
{
    YOGI_ASSERT(m_buffer.size() == sizeof(std::uint32_t));

    std::uint32_t size;
    std::copy(m_buffer.begin(), m_buffer.end(), reinterpret_cast<char*>(&size));
    size = ntohl(size);

    if (size <= YOGI_MAX_TCP_IDENTIFICATION_SIZE) {
        m_buffer.resize(size);
        start_async_receive_identification_data();
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Invalid identification size received: "
            << size << " bytes";
        handle_protocol_error<YOGI_ERR_IDENTIFICATION_TOO_LARGE>();
    }
}

void TcpConnectionFactory::start_async_receive_identification_data()
{
    boost::asio::async_read(m_socket, boost::asio::buffer(m_buffer),
        [=](const boost::system::error_code& ec, std::size_t) {
            completion_handler(ec, "read", &TcpConnectionFactory::
                on_receive_identification_data_succeeded);
        }
    );
}

void TcpConnectionFactory::on_receive_identification_data_succeeded()
{
    auto lock = make_lock_guard();

    if (m_firstResult) {
        on_shake_hands_completed(*m_firstResult, tcp_connection_ptr{});
        m_socket.close();
    }
    else {
        m_firstResult = std::make_unique<api::ExceptionT<YOGI_OK>>();
        m_timer.cancel();
    }
}

void TcpConnectionFactory::on_timeout(const boost::system::error_code& ec)
{
    auto lock = make_lock_guard();

    if (m_firstResult) {
        tcp_connection_ptr conn;
        if (m_firstResult->error_code() == YOGI_OK) {
            conn = make_connection();
        }
        else {
            m_socket.close();
        }
        on_shake_hands_completed(*m_firstResult, std::move(conn));
    }
    else {
        // timeout
        if (!ec) {
            m_firstResult = std::make_unique<api::ExceptionT<
                YOGI_ERR_TIMEOUT>>();
        }
        // canceled (or some other error)
        else {
            m_firstResult = std::make_unique<api::ExceptionT<
                YOGI_ERR_CANCELED>>();
        }

        cancel_socket();
    }
}

TcpConnectionFactory::TcpConnectionFactory(interfaces::IScheduler& scheduler,
    identification_buffer identification)
    : m_scheduler     {scheduler.make_ptr<interfaces::IScheduler>()}
    , m_identification{make_identification(identification)}
    , m_socket        {scheduler.io_service()}
    , m_timer         {scheduler.io_service()}
{
}

std::unique_lock<std::recursive_mutex> TcpConnectionFactory::make_lock_guard()
{
    return std::unique_lock<std::recursive_mutex>{m_mutex};
}

void TcpConnectionFactory::start_async_shake_hands(
    boost::asio::ip::tcp::socket&& socket,
    std::chrono::milliseconds rcvTimeout)
{
    auto lock = make_lock_guard();

    m_firstResult.reset();
    m_socket = std::move(socket);
    m_canceled = false;

    boost::system::error_code ec;
    m_socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(warning) << "Could not set no_delay option on "
            << "socket: " << ec.message();
    }

    start_async_send_header();

    if (rcvTimeout == rcvTimeout.max()) {
        m_timer.expires_from_now(boost::posix_time::pos_infin);
    }
    else {
        m_timer.expires_from_now(boost::posix_time::milliseconds(
            rcvTimeout.count()), ec);
    }

    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Could not start timer: "
            << ec.message();
        cancel_socket();
    }

    m_timer.async_wait(
        [=](const boost::system::error_code& ec) {
            on_timeout(ec);
        }
    );
}

void TcpConnectionFactory::cancel_shake_hands()
{
    auto lock = make_lock_guard();
    cancel_socket();
}

} // namespace tcp
} // namespace connections
} // namespace yogi
