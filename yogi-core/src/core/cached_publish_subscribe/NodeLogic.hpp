#ifndef YOGI_CORE_CACHED_PUBLISH_SUBSCRIBE_NODELOGIC_HPP
#define YOGI_CORE_CACHED_PUBLISH_SUBSCRIBE_NODELOGIC_HPP

#include "../../config.h"
#include "../common/PublishSubscribeNodeLogicBaseT.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace cached_publish_subscribe {

/***************************************************************************//**
 * Implements the logic for publish-subscribe terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class NodeLogic : public common::PublishSubscribeNodeLogicBaseT<TTypes>
{
    typedef common::PublishSubscribeNodeLogicBaseT<TTypes> super;

private:
    void sendCachedData(interfaces::IConnection& connection,
        base::Id subscriptionId, const typename super::terminal_info& tm)
    {
        using namespace messaging;

        if (tm.ext.lastReceivedMessageSet) {
            typename TTypes::CachedData msg;
            msg[fields::subscriptionId] = subscriptionId;
            msg[fields::data]           = tm.ext.lastReceivedMessage;

            connection.send(msg);
        }
    }

protected:
    NodeLogic(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
        super::template add_msg_handler<typename TTypes::CachedData>(this,
            &NodeLogic::on_message_received);
    }

    void on_message_received(typename TTypes::CachedData&& msg,
        interfaces::IConnection& origin)
    {
        using namespace messaging;

        auto& tm = super::get_terminal_info(msg[fields::subscriptionId]);
        tm.ext.lastReceivedMessage    = msg[fields::data];
        tm.ext.lastReceivedMessageSet = true;

        super::template send_data_to_subscribers<typename TTypes::CachedData>(
            std::move(msg[fields::data]), msg[fields::subscriptionId], origin);
    }

    virtual void on_data_received(base::Buffer&& data,
        base::Id terminalId, interfaces::IConnection& origin) override
    {
        auto& tm = super::get_terminal_info(terminalId);
        tm.ext.lastReceivedMessage    = data;
        tm.ext.lastReceivedMessageSet = true;

        super::on_data_received(std::move(data), terminalId, origin);
    }

    virtual void on_subscribed(interfaces::IConnection& origin,
        base::Id subscriptionId, const typename super::terminal_info& tm)
        override
    {
        sendCachedData(origin, subscriptionId, tm);
    }

    virtual void on_binding_owner_remapped(interfaces::IConnection& connection,
        typename super::const_binding_iterator bd, base::Id newMappedId) override
    {
        super::on_binding_owner_remapped(connection, bd, newMappedId);

        if (bd->terminal) {
            auto& tm = super::get_terminal_info(bd->terminal);
            sendCachedData(connection, newMappedId, tm);
        }
    }

    virtual void on_binding_owner_added(interfaces::IConnection& connection,
        typename super::const_binding_iterator bd, base::Id mappedId) override
    {
        super::on_binding_owner_added(connection, bd, mappedId);

        if (bd->terminal) {
            auto& tm = super::get_terminal_info(bd->terminal);
            sendCachedData(connection, mappedId, tm);
        }
    }
};

} // namespace cached_publish_subscribe
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_CACHED_PUBLISH_SUBSCRIBE_NODELOGIC_HPP
