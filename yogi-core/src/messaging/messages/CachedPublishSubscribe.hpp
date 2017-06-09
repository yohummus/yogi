#ifndef YOGI_MESSAGING_MESSAGES_CACHEDPUBLISHSUBSCRIBE_HPP
#define YOGI_MESSAGING_MESSAGES_CACHEDPUBLISHSUBSCRIBE_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct CachedPublishSubscribe {
	struct TerminalDescription : public InheritedMessage<TerminalDescription,
        PublishSubscribe::TerminalDescription
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::TerminalDescription"); };

	struct TerminalMapping : public InheritedMessage<TerminalMapping,
        PublishSubscribe::TerminalMapping
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::TerminalMapping"); };

	struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        PublishSubscribe::TerminalNoticed
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::TerminalNoticed"); };

	struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        PublishSubscribe::TerminalRemoved
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::TerminalRemoved"); };

	struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        PublishSubscribe::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::TerminalRemovedAck"); };

	struct BindingDescription : public InheritedMessage<BindingDescription,
        PublishSubscribe::BindingDescription
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingDescription"); };

	struct BindingMapping : public InheritedMessage<BindingMapping,
        PublishSubscribe::BindingMapping
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingMapping"); };

	struct BindingNoticed : public InheritedMessage<BindingNoticed,
        PublishSubscribe::BindingNoticed
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingNoticed"); };

	struct BindingRemoved : public InheritedMessage<BindingRemoved,
        PublishSubscribe::BindingRemoved
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingRemoved"); };

	struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        PublishSubscribe::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingRemovedAck"); };

	struct BindingEstablished : public InheritedMessage<BindingEstablished,
        PublishSubscribe::BindingEstablished
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingEstablished"); };

	struct BindingReleased : public InheritedMessage<BindingReleased,
        PublishSubscribe::BindingReleased
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        PublishSubscribe::Subscribe
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        PublishSubscribe::Unsubscribe
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        PublishSubscribe::Data
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::Data"); };

    struct CachedData : public Message<CachedData,
        fields::SubscriptionId,
        fields::Data
    > { YOGI_MESSAGE_NAME("CachedPublishSubscribe::CachedData"); };
}; // struct CachedPublishSubscribe

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_CACHEDPUBLISHSUBSCRIBE_HPP
