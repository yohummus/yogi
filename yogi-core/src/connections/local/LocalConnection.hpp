#ifndef CHIRP_CONNECTIONS_LOCAL_LOCALCONNECTION_HPP
#define CHIRP_CONNECTIONS_LOCAL_LOCALCONNECTION_HPP

#include "../../config.h"
#include "../../interfaces/ICommunicator.hpp"

#include <boost/asio/strand.hpp>

#include <atomic>


namespace chirp {
namespace connections {
namespace local {

/***************************************************************************//**
 * Provides a connection between nodes/leafs within the same process
 ******************************************************************************/
class LocalConnection : public interfaces::IConnectionLike
{
private:
    class Proxy;

    struct channel_data {
        channel_data*                sender;
        interfaces::communicator_ptr receiver;
        Proxy                       *proxy;
        bool                         remoteIsNode;
        boost::asio::strand          strand;
        std::atomic<bool>            delayDeath;
        
        channel_data(interfaces::ICommunicator& receiver_);
    };

    enum state_t {
        STATE_REGISTRATION,
        STATE_RUNNING,
        STATE_CLOSED
    };

    /***********************************************************************//**
     * Implements the connection interface for one side of the connection
     **************************************************************************/
    class Proxy : public interfaces::IConnection
    {
    private:
        LocalConnection& m_connection;
        channel_data&    m_channel;

    public:
        Proxy(LocalConnection& connection, channel_data& channel);

        virtual void send(const interfaces::IMessage& msg) override;
        virtual bool remote_is_node() const override;
        virtual const std::string& description() const override;
		virtual const std::string& remote_version() const override;
		virtual const std::vector<char>& remote_identification() const override;
    };

private:
    channel_data         m_channelAtoB;
    channel_data         m_channelBtoA;
    Proxy                m_proxyA;
    Proxy                m_proxyB;
    std::atomic<state_t> m_state;
    std::atomic<int>     m_activePosts;

private:
    void deliver_death(channel_data& channel);
    void deliver_msg(const interfaces::message_ptr& msg, channel_data& channel);
    void post_death(channel_data& channel);
    void post_msg(const interfaces::IMessage& msg, channel_data& channel);
    void register_channels();
    void await_idle();
    bool remote_is_node(const channel_data& channel) const;
    void close();

public:
    LocalConnection(interfaces::ICommunicator& sideA,
        interfaces::ICommunicator& sideB);
    virtual ~LocalConnection();

    virtual const std::string& description() const override;
    virtual const std::string& remote_version() const override;
    virtual const std::vector<char>& remote_identification() const override;
};

} // namespace local
} // namespace connections
} // namespace chirp

#endif // CHIRP_CONNECTIONS_LOCAL_LOCALCONNECTION_HPP
