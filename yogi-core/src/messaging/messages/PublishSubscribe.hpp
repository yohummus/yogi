#ifndef CHIRP_MESSAGING_MESSAGES_PUBLISHSUBSCRIBE_HPP
#define CHIRP_MESSAGING_MESSAGES_PUBLISHSUBSCRIBE_HPP

#include "../../config.h"
#include "../InheritedMessage.hpp"
#include "DeafMute.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct PublishSubscribe {
	struct TerminalDescription : public InheritedMessage<TerminalDescription,
        DeafMute::TerminalDescription
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::TerminalDescription"); };

	struct TerminalMapping : public InheritedMessage<TerminalMapping,
        DeafMute::TerminalMapping
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::TerminalMapping"); };

	struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        DeafMute::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::TerminalNoticed"); };

	struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        DeafMute::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::TerminalRemoved"); };

	struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        DeafMute::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::TerminalRemovedAck"); };

	struct BindingDescription : public InheritedMessage<BindingDescription,
        DeafMute::BindingDescription
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingDescription"); };

	struct BindingMapping : public InheritedMessage<BindingMapping,
        DeafMute::BindingMapping
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingMapping"); };

	struct BindingNoticed : public InheritedMessage<BindingNoticed,
        DeafMute::BindingNoticed
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingNoticed"); };

	struct BindingRemoved : public InheritedMessage<BindingRemoved,
        DeafMute::BindingRemoved
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingRemoved"); };

	struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        DeafMute::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingRemovedAck"); };

	struct BindingEstablished : public InheritedMessage<BindingEstablished,
        DeafMute::BindingEstablished
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingEstablished"); };

	struct BindingReleased : public InheritedMessage<BindingReleased,
        DeafMute::BindingReleased
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::BindingReleased"); };

	struct Subscribe : public Message<Subscribe,
		fields::TerminalId
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::Subscribe"); };

	struct Unsubscribe : public Message<Unsubscribe,
		fields::TerminalId
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::Unsubscribe"); };

	struct Data : public Message<Data,
		fields::SubscriptionId,
		fields::Data
    > { CHIRP_MESSAGE_NAME("PublishSubscribe::Data"); };
}; // struct PublishSubscribe

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_PUBLISHSUBSCRIBE_HPP
