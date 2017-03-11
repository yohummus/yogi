#include "TcpConnection.hpp"
#include "../../interfaces/INode.hpp"
#include "../../yogi_core.h"
#include "../../serialization/serialize.hpp"
#include "../../serialization/deserialize.hpp"
#include "../../serialization/can_deserialize_one.hpp"
#include "../../messaging/MessageRegister.hpp"
#include "../../base/AsyncOperation.hpp"
#include "../../yogi_core.h"

#include <boost/log/trivial.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <thread>


namespace yogi {
namespace connections {
namespace tcp {

std::string TcpConnection::make_description(
    const boost::asio::ip::tcp::socket& s)
{
    try {
        return s.remote_endpoint().address().to_string() + ":" +
            std::to_string(s.remote_endpoint().port());
    }
    catch (const boost::system::system_error&) {
        return "UNCONNECTED";
    }
}

void TcpConnection::die(const boost::system::error_code& ec, bool* runningFlag)
{
    std::lock_guard<std::recursive_mutex> lock{m_mutex};

    close_socket();

    *runningFlag = false;

    if (m_alive) {
        m_alive = false;

        if (ec == boost::asio::error::eof) {
            m_awaitDeathOp.fire<YOGI_ERR_CONNECTION_CLOSED>();
        }
        else {
            m_awaitDeathOp.fire<YOGI_ERR_SOCKET_BROKEN>();
        }
    }

    m_cv.notify_all();
}

void TcpConnection::done(bool* runningFlag)
{
    YOGI_ASSERT(*runningFlag);
    *runningFlag = false;
    m_cv.notify_all();
}

void TcpConnection::start_send_communicator_type()
{
    auto data = std::make_shared<char>(
        std::dynamic_pointer_cast<interfaces::INode>(m_communicator) ? 1 : 0);

    boost::asio::async_write(m_socket, boost::asio::buffer(data.get(), 1),
        [=](const boost::system::error_code& ec, std::size_t) {
        auto _ = data; // just so data does not get destroyed too early
        on_send_communicator_type_completed(ec);
    }
    );

    m_preMessagingRunning = true;
}

void TcpConnection::on_send_communicator_type_completed(
    const boost::system::error_code& ec)
{
    if (!ec) {
        std::lock_guard<std::recursive_mutex> lock{m_mutex};
        start_receive_communicator_type();
    }
    else {
        BOOST_LOG_TRIVIAL(error) << m_description << ": Async send communicator"
            " type failed: " << ec.message();
        die(ec, &m_preMessagingRunning);
    }
}

void TcpConnection::start_receive_communicator_type()
{
    auto data = std::make_shared<char>();
    boost::asio::async_read(m_socket, boost::asio::buffer(data.get(), 1),
        [=](const boost::system::error_code& ec, std::size_t) {
        on_receive_communicator_type_completed(ec, data);
    }
    );
}

void TcpConnection::on_receive_communicator_type_completed(
    const boost::system::error_code& ec, std::shared_ptr<char> data)
{
    if (!ec) {
        std::unique_lock<std::mutex> rcvLock{m_receiveMutex};
        std::lock_guard<std::recursive_mutex> lock{m_mutex};

        m_remoteIsNode = *data ? true : false;
        m_ready = true;

        if (m_communicator) {
            m_communicator->on_connection_started(*this);
        }

        start_async_receive_some_data();

        done(&m_preMessagingRunning);
    }
    else {
        BOOST_LOG_TRIVIAL(error) << m_description << ": Async receive "
            "communicator type failed: " << ec.message();
        die(ec, &m_preMessagingRunning);
    }
}

void TcpConnection::start_async_receive_some_data()
{
    if (m_recvSomeDataRunning) {
        return;
    }

    m_socket.async_receive(m_inBuffer.first_write_array(),
        [=](const boost::system::error_code& ec, std::size_t bytesReceived) {
            on_receive_some_data_completed(ec, bytesReceived);
        }
    );

    m_recvSomeDataRunning = true;
}

void TcpConnection::on_receive_some_data_completed(
    const boost::system::error_code& ec, std::size_t bytesReceived)
{
    if (!ec) {
        std::lock_guard<std::mutex> lock{m_receiveMutex};

        m_heartbeatsSinceLastReceive = 0;

        m_inBuffer.commit_first_write_array(bytesReceived);
        start_async_deserialization();

        if (m_inBuffer.full()) {
            done(&m_recvSomeDataRunning);
        }
        else {
            m_recvSomeDataRunning = false;
            start_async_receive_some_data();
            m_cv.notify_all(); // tell threads that we received data
        }
    }
    else {
        BOOST_LOG_TRIVIAL(error) << m_description << ": Async receive some data"
            " failed: " << ec.message();

        std::lock_guard<std::mutex> lock{m_receiveMutex};
        die(ec, &m_recvSomeDataRunning);
    }
}

void TcpConnection::start_async_send_some_data()
{
    if (m_sendSomeDataRunning) {
        return;
    }

    m_socket.async_send(m_outBuffer.first_read_array(),
        [=](const boost::system::error_code& ec, std::size_t bytesSent) {
        on_send_some_data_completed(ec, bytesSent);
    }
    );

    m_sendSomeDataRunning = true;
}

void TcpConnection::on_send_some_data_completed(
    const boost::system::error_code& ec, std::size_t bytesSent)
{
    if (!ec) {
        std::unique_lock<std::recursive_mutex> lock{m_mutex};

        m_heartbeatsSinceLastSend = 0;

        m_outBuffer.commit_first_read_array(bytesSent);
        if (m_outBuffer.empty()) {
            done(&m_sendSomeDataRunning);
        }
        else {
            m_sendSomeDataRunning = false;
            start_async_send_some_data();
            m_cv.notify_all(); // tell threads that we sent some data
        }
    }
    else {
        die(ec, &m_sendSomeDataRunning);
    }
}

void TcpConnection::start_async_deserialization()
{
    if (m_deserializeRunning || !m_alive) {
        return;
    }

    m_communicator->scheduler().post([&] {
        deserialize();
    });

    m_deserializeRunning = true;
}

void TcpConnection::deserialize()
{
    while (true) {
        // read message size
        if (m_remainingMsgPayload == 0) {
            while (!serialization::can_deserialize_one<std::size_t>(
                m_tmpInBuffer, m_tmpInBuffer.begin()))
            {
                if (m_inBuffer.empty()) {
                    wait_for_more_data_to_deserialize();
                    return;
                }
                else {
                    m_tmpInBuffer.push_back(m_inBuffer.front());
                    m_inBuffer.pop();
                }
            }

            serialization::deserialize(m_tmpInBuffer, m_tmpInBuffer.begin(),
                m_remainingMsgPayload);
            m_tmpInBuffer.resize(m_remainingMsgPayload);
        }

        // did we receive a heartbeat?
        if (m_remainingMsgPayload == 0) {
            continue;
        }

        // read message type ID and payload
        m_remainingMsgPayload -= m_inBuffer.read(m_tmpInBuffer.data()
            + m_tmpInBuffer.size() - m_remainingMsgPayload,
            m_remainingMsgPayload);

        if (m_remainingMsgPayload > 0) {
            wait_for_more_data_to_deserialize();
            return;
        }

        deserialize_message_and_forward_to_communicator();
        m_tmpInBuffer.clear();
    }
}

void TcpConnection::wait_for_more_data_to_deserialize()
{
    std::lock_guard<std::mutex> lock{m_receiveMutex};
    start_async_receive_some_data();
    m_deserializeRunning = false;
    m_cv.notify_all();
}

void TcpConnection::deserialize_message_and_forward_to_communicator()
{
    interfaces::IMessage::id_type msgTypeId;
    auto it = serialization::deserialize(m_tmpInBuffer, m_tmpInBuffer.begin(),
        msgTypeId);

    std::lock_guard<std::mutex> lock{m_receiveMutex};
    if (m_alive) {
        messaging::MessageRegister::deserialize_and_forward_message(msgTypeId,
            m_tmpInBuffer, it, *m_communicator, *this);
    }
}

void TcpConnection::start_async_wait()
{
    if (m_timerRunning) {
        return;
    }

    boost::system::error_code ec;
    m_timer.expires_from_now(boost::posix_time::milliseconds(
        m_timeout.count() / 2), ec);

    if (ec) {
        BOOST_LOG_TRIVIAL(error) << m_description << "Could not start timer: "
            << ec.message();
    }

    m_timer.async_wait(
        [=](const boost::system::error_code& ec) {
            on_timeout(ec);
        });

    m_timerRunning = true;
}

void TcpConnection::on_timeout(const boost::system::error_code& ec)
{
    std::unique_lock<std::recursive_mutex> lock{m_mutex};

    if (!ec) {
        // receive timeout
        if (m_heartbeatsSinceLastReceive >= 2) {
            BOOST_LOG_TRIVIAL(error) << m_description << ": Connection timed "
                "out";

            close_socket();
            m_alive = false;

            m_awaitDeathOp.fire<YOGI_ERR_TIMEOUT>();

            done(&m_timerRunning);
        }
        // send heartbeat
        else {
            if (m_alive) {
                ++m_heartbeatsSinceLastReceive;

                if (m_heartbeatsSinceLastSend >= 1) {
                    send_heartbeat(lock);
                }
                else {
                    ++m_heartbeatsSinceLastSend;
                }

                m_timerRunning = false;
                start_async_wait();
            }
            else {
                done(&m_timerRunning);
            }
        }
    }
    else {
        if (ec == boost::asio::error::operation_aborted) {
            m_awaitDeathOp.fire<YOGI_ERR_CANCELED>();
        }
        else {
            BOOST_LOG_TRIVIAL(error) << m_description << ": Async wait "
                "operation failed: " << ec.message();
        }

        done(&m_timerRunning);
    }
}

void TcpConnection::close_socket()
{
    boost::system::error_code ec;
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
}

void TcpConnection::send_heartbeat(std::unique_lock<std::recursive_mutex>& lock)
{
    static const std::vector<char> buffer{0};
    auto it = m_outBuffer.write(buffer.begin(), buffer.end());

    if (it != buffer.end()) {
        start_async_send_some_data();
    }

    m_cv.wait(lock, [&] {
        return !m_alive || m_outBuffer.write(it, buffer.end())
            == buffer.end();
    });
}

TcpConnection::TcpConnection(interfaces::IScheduler& scheduler,
    boost::asio::ip::tcp::socket&& socket, std::string remoteVersion,
    std::vector<char> remoteIdentification)
    : m_scheduler                 {scheduler.make_ptr<interfaces::IScheduler>()}
    , m_remoteVersion             {remoteVersion}
    , m_remoteIdentification      {remoteIdentification}
    , m_description               {make_description(socket)}
    , m_socket                    {std::move(socket)}
    , m_alive                     {true}
    , m_ready                     {false}
    , m_remainingMsgPayload       {0}
    , m_preMessagingRunning       {false}
    , m_sendSomeDataRunning       {false}
    , m_recvSomeDataRunning       {false}
    , m_deserializeRunning        {false}
    , m_timer                     {scheduler.io_service()}
    , m_timerRunning              {false}
    , m_heartbeatsSinceLastReceive{0}
    , m_heartbeatsSinceLastSend   {0}
{
    BOOST_LOG_TRIVIAL(info) << "TCP connection to " << m_description
        << " running YOGI " << m_remoteVersion << " successfully created";
}

TcpConnection::~TcpConnection()
{
    interfaces::communicator_ptr communicator;

    {{
        std::unique_lock<std::mutex> rcvLock{m_receiveMutex};
        std::unique_lock<std::recursive_mutex> lock{m_mutex};
        m_alive = false;
        m_cv.notify_all();
        std::swap(m_communicator, communicator);
    }}

    if (communicator) {
        communicator->on_connection_destroyed(*this);
    }

    {{
        std::unique_lock<std::recursive_mutex> lock{m_mutex};

        boost::system::error_code ec;
        close_socket();
        m_timer.cancel(ec);

        cancel_await_death();
        m_awaitDeathOp.await_idle();

        m_cv.wait(lock, [&] {
            return !m_preMessagingRunning
                && !m_sendSomeDataRunning
                && !m_timerRunning;
        });
    }}

    {{
        std::unique_lock<std::mutex> lock{m_receiveMutex};
        m_cv.wait(lock, [&] {
            return !m_recvSomeDataRunning
                && !m_deserializeRunning;
        });
    }}
}

void TcpConnection::assign(interfaces::ICommunicator& communicator,
    std::chrono::milliseconds timeout)
{
    std::lock_guard<std::recursive_mutex> lock{m_mutex};

    if (m_communicator) {
        throw api::ExceptionT<YOGI_ERR_ALREADY_ASSIGNED>{};
    }

    if (!m_alive) {
        throw api::ExceptionT<YOGI_ERR_CONNECTION_DEAD>{};
    }

    if (timeout.count() / 2 == 0) {
        throw api::ExceptionT<YOGI_ERR_INVALID_PARAM>{};
    }

    if (timeout != timeout.max()) {
        m_timeout = timeout;
        start_async_wait();
    }

    communicator.on_new_connection(*this);
    m_communicator = communicator.make_ptr<interfaces::ICommunicator>();

    start_send_communicator_type();
}

void TcpConnection::async_await_death(error_handler_fn handlerFn)
{
    std::lock_guard<std::recursive_mutex> lock{m_mutex};

    if (!m_alive) {
        throw api::ExceptionT<YOGI_ERR_CONNECTION_DEAD>{};
    }
    else {
        m_awaitDeathOp.arm(handlerFn);
    }
}

void TcpConnection::cancel_await_death()
{
    std::lock_guard<std::recursive_mutex> lock{m_mutex};
    m_awaitDeathOp.fire<YOGI_ERR_CANCELED>();
}

void TcpConnection::send(const interfaces::IMessage& msg)
{
    std::unique_lock<std::recursive_mutex> lock{m_mutex};

    if (!m_alive) {
        return;
    }

    m_tmpMsgBuffer.clear();
    msg.serialize(m_tmpMsgBuffer);

    m_tmpMsgTypeIdBuffer.clear();
    serialization::serialize(m_tmpMsgTypeIdBuffer, msg.type_id());

    m_tmpMsgSizeBuffer.clear();
    serialization::serialize(m_tmpMsgSizeBuffer, m_tmpMsgBuffer.size()
        + m_tmpMsgTypeIdBuffer.size());

    m_tmpHeaderBuffer.clear();
    m_tmpHeaderBuffer.insert(m_tmpHeaderBuffer.end(),
        m_tmpMsgSizeBuffer.begin(), m_tmpMsgSizeBuffer.end());
    m_tmpHeaderBuffer.insert(m_tmpHeaderBuffer.end(),
        m_tmpMsgTypeIdBuffer.begin(), m_tmpMsgTypeIdBuffer.end());

    auto it = m_outBuffer.write(m_tmpHeaderBuffer.begin(),
        m_tmpHeaderBuffer.end());

    if (it != m_tmpHeaderBuffer.end()) {
        start_async_send_some_data();
    }

    m_cv.wait(lock, [&] {
        return !m_alive || m_outBuffer.write(it, m_tmpHeaderBuffer.end())
            == m_tmpHeaderBuffer.end();
    });

    it = m_outBuffer.write(m_tmpMsgBuffer.begin(), m_tmpMsgBuffer.end());
    start_async_send_some_data();

    m_cv.wait(lock, [&] {
        return !m_alive || m_outBuffer.write(it, m_tmpMsgBuffer.end())
            == m_tmpMsgBuffer.end();
    });
}

bool TcpConnection::remote_is_node() const
{
    if (!m_ready) {
        throw api::ExceptionT<YOGI_ERR_NOT_READY>{};
    }

    return m_remoteIsNode;
}

const std::string& TcpConnection::description() const
{
    return m_description;
}

const std::string& TcpConnection::remote_version() const
{
	return m_remoteVersion;
}

const std::vector<char>& TcpConnection::remote_identification() const
{
	return m_remoteIdentification;
}

} // namespace tcp
} // namespace connections
} // namespace yogi
