#ifndef YOGI_MESSAGING_MESSAGES_PUBLISHSUBSCRIBE_HPP
#define YOGI_MESSAGING_MESSAGES_PUBLISHSUBSCRIBE_HPP

#include "../../config.h"
#include "../InheritedMessage.hpp"
#include "DeafMute.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct PublishSubscribe {
	struct TerminalDescription : public InheritedMessage<TerminalDescription,
        DeafMute::TerminalDescription
    > { YOGI_MESSAGE_NAME("PublishSubscribe::TerminalDescription"); };

	struct TerminalMapping : public InheritedMessage<TerminalMapping,
        DeafMute::TerminalMapping
    > { YOGI_MESSAGE_NAME("PublishSubscribe::TerminalMapping"); };

	struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        DeafMute::TerminalNoticed
    > { YOGI_MESSAGE_NAME("PublishSubscribe::TerminalNoticed"); };

	struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        DeafMute::TerminalRemoved
    > { YOGI_MESSAGE_NAME("PublishSubscribe::TerminalRemoved"); };

	struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        DeafMute::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("PublishSubscribe::TerminalRemovedAck"); };

	struct BindingDescription : public InheritedMessage<BindingDescription,
        DeafMute::BindingDescription
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingDescription"); };

	struct BindingMapping : public InheritedMessage<BindingMapping,
        DeafMute::BindingMapping
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingMapping"); };

	struct BindingNoticed : public InheritedMessage<BindingNoticed,
        DeafMute::BindingNoticed
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingNoticed"); };

	struct BindingRemoved : public InheritedMessage<BindingRemoved,
        DeafMute::BindingRemoved
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingRemoved"); };

	struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        DeafMute::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingRemovedAck"); };

	struct BindingEstablished : public InheritedMessage<BindingEstablished,
        DeafMute::BindingEstablished
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingEstablished"); };

	struct BindingReleased : public InheritedMessage<BindingReleased,
        DeafMute::BindingReleased
    > { YOGI_MESSAGE_NAME("PublishSubscribe::BindingReleased"); };

	struct Subscribe : public Message<Subscribe,
		fields::TerminalId
    > { YOGI_MESSAGE_NAME("PublishSubscribe::Subscribe"); };

	struct Unsubscribe : public Message<Unsubscribe,
		fields::TerminalId
    > { YOGI_MESSAGE_NAME("PublishSubscribe::Unsubscribe"); };

	struct Data : public Message<Data,
		fields::SubscriptionId,
		fields::Data
    > { YOGI_MESSAGE_NAME("PublishSubscribe::Data"); };
}; // struct PublishSubscribe

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_PUBLISHSUBSCRIBE_HPP
