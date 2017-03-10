#ifndef CHIRP_CORE_COMMON_PUBLISHSUBSCRIBENODELOGICBASET_HPP
#define CHIRP_CORE_COMMON_PUBLISHSUBSCRIBENODELOGICBASET_HPP

#include "../../config.h"
#include "SubscribableNodeLogicBaseT.hpp"


namespace chirp {
namespace core {
namespace common {

/***************************************************************************//**
 * Implements the logic for publish-subscribe terminals on leafs
 ******************************************************************************/
template <typename TTypes>
class PublishSubscribeNodeLogicBaseT : public SubscribableNodeLogicBaseT<TTypes>
{
    typedef SubscribableNodeLogicBaseT<TTypes> super;

protected:
    PublishSubscribeNodeLogicBaseT(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
        super::template add_msg_handler<typename TTypes::Data>(this,
            &PublishSubscribeNodeLogicBaseT::on_message_received);
    }
    
    void on_message_received(typename TTypes::Data&& msg,
        interfaces::IConnection& origin)
    {
        using namespace messaging;

        on_data_received(std::move(msg[fields::data]),
            msg[fields::subscriptionId], origin);
    }

    virtual void on_data_received(base::Buffer&& data,
        base::Id terminalId, interfaces::IConnection& origin)
    {
        send_data_to_subscribers<typename TTypes::Data>(std::move(data),
            terminalId, origin);
    }

    template <typename TMsg>
    void send_data_to_subscribers(base::Buffer&& data,
        base::Id terminalId, interfaces::IConnection& origin)
    {
        using namespace messaging;

        TMsg fwMsg;
        fwMsg[fields::data] = std::move(data);

        auto& tm = super::get_terminal_info(terminalId);
        for (auto& subscriber : tm.subscribers) {
            auto conn = const_cast<interfaces::IConnection*>(subscriber.first);
            if (conn != &origin) {
                CHIRP_ASSERT(tm.usingNodes.count(conn));
                CHIRP_ASSERT(tm.usingNodes.find(conn)->second.is_mapped());
                fwMsg[fields::subscriptionId] = subscriber.second;
                conn->send(fwMsg);
            }
        }

        if (tm.binding) {
            auto& bd = super::get_binding_info(tm.binding);
            for (auto& owner : bd.owningLeafs) {
                if (owner.first != &origin) {
                    CHIRP_ASSERT(owner.second.is_mapped());
                    fwMsg[fields::subscriptionId] = owner.second.mapped_id();
                    owner.first->send(fwMsg);
                }
            }
        }
    }
};

} // namespace common
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_COMMON_PUBLISHSUBSCRIBENODELOGICBASET_HPP
