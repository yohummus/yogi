#ifndef YOGI_CORE_COMMON_PUBLISHSUBSCRIBELEAFLOGICBASET_HPP
#define YOGI_CORE_COMMON_PUBLISHSUBSCRIBELEAFLOGICBASET_HPP

#include "../../config.h"
#include "SubscribableLeafLogicBaseT.hpp"


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Templated base class for leaf logic implementations based on the publish-
 * subscribe messaging pattern
 ******************************************************************************/
template <typename TTypes>
class PublishSubscribeLeafLogicBaseT : public SubscribableLeafLogicBaseT<TTypes>
{
    typedef SubscribableLeafLogicBaseT<TTypes> super;

protected:
    PublishSubscribeLeafLogicBaseT(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
        super::template add_msg_handler<typename TTypes::Data>(this,
            &PublishSubscribeLeafLogicBaseT::on_message_received);
    }

    void on_message_received(typename TTypes::Data&& msg)
    {
		using namespace messaging;

        auto& bindings = super::get_binding_info(msg[fields::subscriptionId])
			.bindings;
        if (bindings.empty()) {
            return;
        }

        auto& info = super::get_terminal_info(bindings[0]->terminal().id());
        on_data_received(info, std::move(msg[fields::data]));
    }

    virtual void on_data_received(const typename super::terminal_info& tm,
        base::Buffer&& data)
    {
        tm.terminal->on_data_received(std::move(data), false);
    }

    virtual void on_data_published(const typename super::terminal_info& tm,
        base::Buffer&& data)
    {
    }

public:
    bool publish(typename TTypes::terminal_type& terminal,
        base::Buffer&& data)
    {
		using namespace messaging;

        auto lock = super::make_lock_guard();

        bool dataSent = false;
        auto& tm = super::get_terminal_info(terminal.id());
        if (tm.subscribed) {
            typename TTypes::Data msg;
            msg[fields::subscriptionId] = tm.fsm.mapped_id();
            msg[fields::data]           = data;

            super::connection().send(msg);
            dataSent = true;
        }

        on_data_published(tm, std::move(data));

        return dataSent;
    }
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_PUBLISHSUBSCRIBELEAFLOGICBASET_HPP
