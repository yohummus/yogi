#ifndef YOGI_MESSAGING_MESSAGES_SCATTERGATHER_HPP
#define YOGI_MESSAGING_MESSAGES_SCATTERGATHER_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct ScatterGather {
	struct TerminalDescription : public InheritedMessage<TerminalDescription,
		DeafMute::TerminalDescription
    > { YOGI_MESSAGE_NAME("ScatterGather::TerminalDescription"); };

	struct TerminalMapping : public InheritedMessage<TerminalMapping,
		DeafMute::TerminalMapping
    > { YOGI_MESSAGE_NAME("ScatterGather::TerminalMapping"); };

	struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
		DeafMute::TerminalNoticed
    > { YOGI_MESSAGE_NAME("ScatterGather::TerminalNoticed"); };

	struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
		DeafMute::TerminalRemoved
    > { YOGI_MESSAGE_NAME("ScatterGather::TerminalRemoved"); };

	struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
		DeafMute::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("ScatterGather::TerminalRemovedAck"); };

	struct BindingDescription : public InheritedMessage<BindingDescription,
		DeafMute::BindingDescription
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingDescription"); };

	struct BindingMapping : public InheritedMessage<BindingMapping,
		DeafMute::BindingMapping
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingMapping"); };

	struct BindingNoticed : public InheritedMessage<BindingNoticed,
		DeafMute::BindingNoticed
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingNoticed"); };

	struct BindingRemoved : public InheritedMessage<BindingRemoved,
		DeafMute::BindingRemoved
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingRemoved"); };

	struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
		DeafMute::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingRemovedAck"); };

	struct BindingEstablished : public InheritedMessage<BindingEstablished,
		DeafMute::BindingEstablished
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingEstablished"); };

	struct BindingReleased : public InheritedMessage<BindingReleased,
		DeafMute::BindingReleased
    > { YOGI_MESSAGE_NAME("ScatterGather::BindingReleased"); };

	struct Subscribe : public InheritedMessage<Subscribe,
		PublishSubscribe::Subscribe
    > { YOGI_MESSAGE_NAME("ScatterGather::Subscribe"); };

	struct Unsubscribe : public InheritedMessage<Unsubscribe,
		PublishSubscribe::Unsubscribe
    > { YOGI_MESSAGE_NAME("ScatterGather::Unsubscribe"); };

	struct Scatter : public Message<Scatter,
		fields::SubscriptionId,
		fields::OperationId,
		fields::Data
    > { YOGI_MESSAGE_NAME("ScatterGather::Scatter"); };

	struct Gather : public Message<Gather,
		fields::OperationId,
		fields::GatherFlags,
		fields::Data
    > { YOGI_MESSAGE_NAME("ScatterGather::Gather"); };
}; // struct ScatterGather

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_SCATTERGATHER_HPP
