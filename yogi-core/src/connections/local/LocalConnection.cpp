#include "LocalConnection.hpp"

#include "../../core/Node.hpp"
#include "../../core/Leaf.hpp"

#include <boost/log/trivial.hpp>

#include <thread>
#include <chrono>


namespace yogi {
namespace connections {
namespace local {

LocalConnection::channel_data::channel_data(
    interfaces::ICommunicator& receiver_)
    : sender      {}
    , receiver    {receiver_.make_ptr<interfaces::ICommunicator>()}
    , proxy       {}
    , remoteIsNode{!!std::dynamic_pointer_cast<core::Node>(receiver)}
    , strand      {receiver_.scheduler().io_service()}
    , delayDeath  {true}
{
}

LocalConnection::Proxy::Proxy(LocalConnection& connection,
    channel_data& channel)
    : m_connection{connection}
    , m_channel   {channel}
{
}

void LocalConnection::Proxy::send(const interfaces::IMessage& msg)
{
    m_connection.post_msg(msg, m_channel);
}

bool LocalConnection::Proxy::remote_is_node() const
{
    return m_connection.remote_is_node(m_channel);
}

const std::string& LocalConnection::Proxy::description() const
{
    return m_connection.description();
}

const std::string& LocalConnection::Proxy::remote_version() const
{
    return m_connection.remote_version();
}

const std::vector<char>& LocalConnection::Proxy::remote_identification() const
{
	return m_connection.remote_identification();
}

void LocalConnection::deliver_death(channel_data& channel)
{
    if (channel.delayDeath) {
        post_death(channel);
    }
    else {
        channel.receiver->on_connection_destroyed(*channel.proxy);
    }

    --m_activePosts;
}

void LocalConnection::deliver_msg(const interfaces::message_ptr& msg,
    channel_data& channel)
{
    if (m_state != STATE_RUNNING) {
        while (m_state == STATE_REGISTRATION) {
            std::this_thread::yield();
        }

        if (m_state == STATE_CLOSED) {
            --m_activePosts;
            return;
        }
    }

    try {
        BOOST_LOG_TRIVIAL(trace) << "RECV from 0x" << std::hex
            << channel.sender->receiver.get() << " to 0x"
            << channel.receiver.get() << ": " << msg->to_string();
    }
    catch (...) {
    }

    try {
        channel.receiver->on_message_received(std::move(*msg),
            *channel.proxy);
    }
    catch (const std::exception& e) {
        try {
            BOOST_LOG_TRIVIAL(error) << "Exception while processing message"
                << " from local connection: " << e.what() << ". Closing "
                << "connection...";
        }
        catch (...) {
        }

        close();
    }

    --m_activePosts;
}

void LocalConnection::post_death(channel_data& channel)
{
    ++m_activePosts;

    while (true) {
        try {
            channel.strand.post([&]{ deliver_death(channel); });
            break;
        }
        catch (...) {
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void LocalConnection::post_msg(const interfaces::IMessage& msg,
    channel_data& channel)
{
    ++m_activePosts;

    try {
        auto clonedMsg = msg.clone();

        try {
            BOOST_LOG_TRIVIAL(trace) << "SENT from 0x" << std::hex
                << channel.sender->receiver.get() << " to 0x"
                << channel.receiver.get() << ": " << msg.to_string();
        }
        catch (...) {
        }

        //channel.strand.post([=, &channel, clonedMsg = std::move(clonedMsg)] {
        //    deliver_msg(clonedMsg, channel);
        //});

        // TODO: this is a workaround the lines above cause Boost to reject
        // the completion handler
        auto rawClonedMsg = clonedMsg.get();
        channel.strand.post([=, &channel] {
            deliver_msg(interfaces::message_ptr(rawClonedMsg), channel);
        });
        clonedMsg.release();
    }
    catch (const std::exception& e) {
        try {
            BOOST_LOG_TRIVIAL(error) << "Exception while sending message over "
                << "local connection: " << e.what() << ". Closing connection"
                << "...";
        }
        catch (...) {
        }

        close();
        --m_activePosts;
    }
}

void LocalConnection::register_channels()
{
    m_state = STATE_REGISTRATION;

    try {
        m_channelAtoB.receiver->on_new_connection(m_proxyB);
        m_channelAtoB.receiver->on_connection_started(m_proxyB);
        m_channelAtoB.delayDeath = false;

        try {
            m_channelBtoA.receiver->on_new_connection(m_proxyA);
            m_channelBtoA.receiver->on_connection_started(m_proxyA);
            m_channelBtoA.delayDeath = false;
        }
        catch (...) {
            post_death(m_channelAtoB);
            throw;
        }
    }
    catch (const std::exception& e) {
        try {
            BOOST_LOG_TRIVIAL(error) << "Exception while assigning new local "
                << "connection to communicator: " << e.what() << ". Closing "
                << "connection...";
        }
        catch (...) {
        }

        m_state = STATE_CLOSED;
        await_idle();

        throw;
    }

    state_t oldState = STATE_REGISTRATION;
    m_state.compare_exchange_strong(oldState, STATE_RUNNING);
}

void LocalConnection::await_idle()
{
    while (m_activePosts) {
        std::this_thread::yield();
    }
}

bool LocalConnection::remote_is_node(const channel_data& channel) const
{
    return channel.remoteIsNode;
}

void LocalConnection::close()
{
    if (m_state.exchange(STATE_CLOSED) != STATE_CLOSED) {
        post_death(m_channelAtoB);
        post_death(m_channelBtoA);
    }
}

LocalConnection::LocalConnection(interfaces::ICommunicator& sideA,
    interfaces::ICommunicator& sideB)
    : m_channelAtoB{sideB}
    , m_channelBtoA{sideA}
    , m_proxyA     {*this, m_channelAtoB}
    , m_proxyB     {*this, m_channelBtoA}
    , m_activePosts{0}
{
    m_channelAtoB.sender = &m_channelBtoA;
    m_channelBtoA.sender = &m_channelAtoB;

    m_channelAtoB.proxy = &m_proxyB;
    m_channelBtoA.proxy = &m_proxyA;

    register_channels();
}

LocalConnection::~LocalConnection()
{
    close();
    await_idle();
}

const std::string& LocalConnection::description() const
{
    static std::string s{"Local Connection"};
    return s;
}

const std::string& LocalConnection::remote_version() const
{
    static std::string s{YOGI_VERSION};
    return s;
}

const std::vector<char>& LocalConnection::remote_identification() const
{
    static std::vector<char> v;
    return v;
}

} // namespace local
} // namespace connections
} // namespace yogi
