#ifndef YOGI_CORE_CACHED_PUBLISH_SUBSCRIBE_LEAFLOGIC_HPP
#define YOGI_CORE_CACHED_PUBLISH_SUBSCRIBE_LEAFLOGIC_HPP

#include "../../config.h"
#include "../common/PublishSubscribeLeafLogicBaseT.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace cached_publish_subscribe {

/***************************************************************************//**
 * Implements the logic for publish-subscribe terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public common::PublishSubscribeLeafLogicBaseT<TTypes>
{
    typedef common::PublishSubscribeLeafLogicBaseT<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
        super::template add_msg_handler<typename TTypes::CachedData>(this,
            &LeafLogic::on_message_received);
    }

    virtual void on_subscribed(const typename super::terminal_info& tm) override
    {
        using namespace messaging;

        super::on_subscribed(tm);

        if (tm.fsm.is_mapped() && tm.ext.lastSentMessageSet) {
            typename TTypes::CachedData msg;
            msg[fields::subscriptionId] = tm.fsm.mapped_id();
            msg[fields::data]           = tm.ext.lastSentMessage;

            super::connection().send(msg);
        }
    }

    virtual void on_data_received(const typename super::terminal_info& tm,
        base::Buffer&& data) override
    {
        tm.ext.lastReceivedMessage    = data;
        tm.ext.lastReceivedMessageSet = true;

        super::on_data_received(tm, std::move(data));
    }

    virtual void on_data_published(const typename super::terminal_info& tm,
        base::Buffer&& data) override
    {
        tm.ext.lastSentMessage    = data;
        tm.ext.lastSentMessageSet = true;

        super::on_data_published(tm, std::move(data));
    }

    void on_message_received(typename TTypes::CachedData&& msg)
    {
        using namespace messaging;

        auto& bindings = super::get_binding_info(msg[fields::subscriptionId])
            .bindings;
        if (bindings.empty()) {
            return;
        }

        auto& info = super::get_terminal_info(bindings[0]->terminal().id());
        on_cached_data_received(info, std::move(msg[fields::data]));
    }

    virtual void on_cached_data_received(const typename super::terminal_info& tm,
        base::Buffer&& data)
    {
        tm.ext.lastReceivedMessage    = data;
        tm.ext.lastReceivedMessageSet = true;

        tm.terminal->on_data_received(std::move(data), true);
    }

public:
    std::pair<base::Buffer, bool> get_cache(
        typename TTypes::terminal_type& terminal)
    {
        auto lock = super::make_lock_guard();

        auto& info = super::get_terminal_info(terminal.id());
        return std::make_pair(info.ext.lastReceivedMessage,
            info.ext.lastReceivedMessageSet);
    }
};

} // namespace cached_publish_subscribe
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_CACHED_PUBLISH_SUBSCRIBE_LEAFLOGIC_HPP
