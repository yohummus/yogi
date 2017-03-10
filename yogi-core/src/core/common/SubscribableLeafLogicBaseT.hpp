#ifndef CHIRP_CORE_COMMON_SUBSCRIBABLELEAFLOGICBASET_HPP
#define CHIRP_CORE_COMMON_SUBSCRIBABLELEAFLOGICBASET_HPP

#include "../../config.h"
#include "../../interfaces/ISubscribableTerminal.hpp"
#include "LeafLogicBaseT.hpp"
#include "subscribable_logic_types.hpp"


namespace chirp {
namespace core {
namespace common {

/***************************************************************************//**
 * Templated base class for leaf logic implementations that support
 * subscriptions
 ******************************************************************************/
template <typename TTypes>
class SubscribableLeafLogicBaseT : public LeafLogicBaseT<TTypes>
{
    typedef LeafLogicBaseT<TTypes> super;

protected:
    SubscribableLeafLogicBaseT(interfaces::IScheduler& scheduler)
        :  super{scheduler}
    {
        super::template add_msg_handler<typename TTypes::Subscribe>(this,
            &SubscribableLeafLogicBaseT::on_message_received);
        super::template add_msg_handler<typename TTypes::Unsubscribe>(this,
            &SubscribableLeafLogicBaseT::on_message_received);
    }

    virtual void on_terminal_mapping_changed(bool isMapped,
        const typename super::terminal_info& tm) override
    {
        if (!isMapped) {
            if (tm.subscribed) {
                tm.subscribed = false;
                on_unsubscribed(tm);
            }
        }
        else {
            if (super::connection().remote_is_node()) {
                tm.subscribed = false;
            }
            else {
                tm.subscribed = true;
                on_subscribed(tm);
            }
        }
    }

    virtual void on_message_received(typename TTypes::Subscribe&& msg)
    {
		using namespace messaging;

        CHIRP_ASSERT(super::connection().remote_is_node());
        CHIRP_ASSERT(!super::get_terminal_info(msg[fields::terminalId])
			.subscribed);

        auto& tm = super::get_terminal_info(msg[fields::terminalId]);
        tm.subscribed = true;
        on_subscribed(tm);
    }

    virtual void on_message_received(typename TTypes::Unsubscribe&& msg)
    {
		using namespace messaging;

        CHIRP_ASSERT(super::connection().remote_is_node());
        CHIRP_ASSERT(super::get_terminal_info(msg[fields::terminalId])
			.subscribed);

        auto& tm = super::get_terminal_info(msg[fields::terminalId]);
        tm.subscribed = false;
        on_unsubscribed(tm);
    }

    virtual void on_subscribed(const typename super::terminal_info& tm)
    {
        if (tm.terminal) {
            tm.terminal->publish_subscription_state(
                interfaces::ISubscribableTerminal::STATE_SUBSCRIBED);
        }
    }

    virtual void on_unsubscribed(const typename super::terminal_info& tm)
    {
        if (tm.terminal) {
            tm.terminal->publish_subscription_state(
                interfaces::ISubscribableTerminal::STATE_UNSUBSCRIBED);
        }
    }
};

} // namespace common
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_COMMON_SUBSCRIBABLELEAFLOGICBASET_HPP
