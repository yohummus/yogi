#ifndef CHIRP_MESSAGING_MESSAGES_SCATTERGATHER_HPP
#define CHIRP_MESSAGING_MESSAGES_SCATTERGATHER_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct ScatterGather {
	struct TerminalDescription : public InheritedMessage<TerminalDescription,
		DeafMute::TerminalDescription
    > { CHIRP_MESSAGE_NAME("ScatterGather::TerminalDescription"); };

	struct TerminalMapping : public InheritedMessage<TerminalMapping,
		DeafMute::TerminalMapping
    > { CHIRP_MESSAGE_NAME("ScatterGather::TerminalMapping"); };

	struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
		DeafMute::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("ScatterGather::TerminalNoticed"); };

	struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
		DeafMute::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("ScatterGather::TerminalRemoved"); };

	struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
		DeafMute::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("ScatterGather::TerminalRemovedAck"); };

	struct BindingDescription : public InheritedMessage<BindingDescription,
		DeafMute::BindingDescription
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingDescription"); };

	struct BindingMapping : public InheritedMessage<BindingMapping,
		DeafMute::BindingMapping
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingMapping"); };

	struct BindingNoticed : public InheritedMessage<BindingNoticed,
		DeafMute::BindingNoticed
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingNoticed"); };

	struct BindingRemoved : public InheritedMessage<BindingRemoved,
		DeafMute::BindingRemoved
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingRemoved"); };

	struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
		DeafMute::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingRemovedAck"); };

	struct BindingEstablished : public InheritedMessage<BindingEstablished,
		DeafMute::BindingEstablished
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingEstablished"); };

	struct BindingReleased : public InheritedMessage<BindingReleased,
		DeafMute::BindingReleased
    > { CHIRP_MESSAGE_NAME("ScatterGather::BindingReleased"); };

	struct Subscribe : public InheritedMessage<Subscribe,
		PublishSubscribe::Subscribe
    > { CHIRP_MESSAGE_NAME("ScatterGather::Subscribe"); };

	struct Unsubscribe : public InheritedMessage<Unsubscribe,
		PublishSubscribe::Unsubscribe
    > { CHIRP_MESSAGE_NAME("ScatterGather::Unsubscribe"); };

	struct Scatter : public Message<Scatter,
		fields::SubscriptionId,
		fields::OperationId,
		fields::Data
    > { CHIRP_MESSAGE_NAME("ScatterGather::Scatter"); };

	struct Gather : public Message<Gather,
		fields::OperationId,
		fields::GatherFlags,
		fields::Data
    > { CHIRP_MESSAGE_NAME("ScatterGather::Gather"); };
}; // struct ScatterGather

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_SCATTERGATHER_HPP
