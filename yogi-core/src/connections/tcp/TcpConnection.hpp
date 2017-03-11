#ifndef YOGI_CONNECTIONS_TCP_TCPCONNECTION_HPP
#define YOGI_CONNECTIONS_TCP_TCPCONNECTION_HPP

#include "../../config.h"
#include "../../interfaces/IScheduler.hpp"
#include "../../interfaces/IConnection.hpp"
#include "../../interfaces/ICommunicator.hpp"
#include "../../base/AsyncOperation.hpp"
#include "../../base/LockFreeRingBuffer.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <mutex>
#include <condition_variable>
#include <functional>


namespace yogi {
namespace connections {
namespace tcp {

/***************************************************************************//**
 * Represents a TCP connection between two nodes or between a node and a leaf
 ******************************************************************************/
class TcpConnection : public interfaces::IConnection
{
    typedef std::function<void (const api::Exception&)>
        error_handler_fn;

private:
    const interfaces::scheduler_ptr        m_scheduler;
    const std::string                      m_remoteVersion;
    const std::vector<char>                m_remoteIdentification;
    const std::string                      m_description;

    mutable std::recursive_mutex           m_mutex;
    mutable std::condition_variable_any    m_cv;
    boost::asio::ip::tcp::socket           m_socket;
    interfaces::communicator_ptr           m_communicator;
    bool                                   m_alive;
    std::atomic<bool>                      m_ready;
    std::atomic<bool>                      m_remoteIsNode;
    base::AsyncOperation<error_handler_fn> m_awaitDeathOp;

    std::mutex                             m_receiveMutex;
    base::LockFreeRingBuffer               m_outBuffer;
    std::vector<char>                      m_tmpMsgTypeIdBuffer;
    std::vector<char>                      m_tmpMsgSizeBuffer;
    std::vector<char>                      m_tmpHeaderBuffer;
    std::vector<char>                      m_tmpMsgBuffer;
    base::LockFreeRingBuffer               m_inBuffer;
    size_t					               m_remainingMsgPayload;
    std::vector<char>                      m_tmpInBuffer;
    bool                                   m_preMessagingRunning;
    bool                                   m_sendSomeDataRunning;
    bool                                   m_recvSomeDataRunning;
    bool                                   m_deserializeRunning;

    std::chrono::milliseconds              m_timeout;
    boost::asio::deadline_timer            m_timer;
    bool                                   m_timerRunning;
    std::atomic<int>                       m_heartbeatsSinceLastReceive;
    int                                    m_heartbeatsSinceLastSend;

private:
    static std::string make_description(const boost::asio::ip::tcp::socket& s);
    void die(const boost::system::error_code& ec, bool* runningFlag);
    void done(bool* runningFlag);
    void start_send_communicator_type();
    void on_send_communicator_type_completed(
        const boost::system::error_code& ec);
    void start_receive_communicator_type();
    void on_receive_communicator_type_completed(
        const boost::system::error_code& ec, std::shared_ptr<char> data);
    void start_async_receive_some_data();
    void on_receive_some_data_completed(const boost::system::error_code& ec,
        std::size_t bytesReceived);
    void start_async_send_some_data();
    void on_send_some_data_completed(const boost::system::error_code& ec,
        std::size_t bytesSent);
    void start_async_deserialization();
    void deserialize();
    void wait_for_more_data_to_deserialize();
    void deserialize_message_and_forward_to_communicator();
    void start_async_wait();
    void on_timeout(const boost::system::error_code& ec);
    void close_socket();
    void send_heartbeat(std::unique_lock<std::recursive_mutex>& lock);

public:
    TcpConnection(interfaces::IScheduler& scheduler,
        boost::asio::ip::tcp::socket&& socket, std::string remoteVersion,
        std::vector<char> remoteIdentification);
    virtual ~TcpConnection();

    void assign(interfaces::ICommunicator& communicator,
        std::chrono::milliseconds timeout);
    void async_await_death(error_handler_fn handlerFn);
    void cancel_await_death();

    virtual void send(const interfaces::IMessage& msg) override;
    virtual bool remote_is_node() const override;
    virtual const std::string& description() const override;
    virtual const std::string& remote_version() const override;
    virtual const std::vector<char>& remote_identification() const override;
};

typedef std::shared_ptr<TcpConnection> tcp_connection_ptr;

} // namespace tcp
} // namespace connections
} // namespace yogi

#endif // YOGI_CONNECTIONS_TCP_TCPCONNECTION_HPP
