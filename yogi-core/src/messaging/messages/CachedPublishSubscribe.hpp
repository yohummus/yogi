#ifndef CHIRP_MESSAGING_MESSAGES_CACHEDPUBLISHSUBSCRIBE_HPP
#define CHIRP_MESSAGING_MESSAGES_CACHEDPUBLISHSUBSCRIBE_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct CachedPublishSubscribe {
	struct TerminalDescription : public InheritedMessage<TerminalDescription,
        PublishSubscribe::TerminalDescription
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::TerminalDescription"); };

	struct TerminalMapping : public InheritedMessage<TerminalMapping,
        PublishSubscribe::TerminalMapping
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::TerminalMapping"); };

	struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        PublishSubscribe::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::TerminalNoticed"); };

	struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        PublishSubscribe::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::TerminalRemoved"); };

	struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        PublishSubscribe::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::TerminalRemovedAck"); };

	struct BindingDescription : public InheritedMessage<BindingDescription,
        PublishSubscribe::BindingDescription
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingDescription"); };

	struct BindingMapping : public InheritedMessage<BindingMapping,
        PublishSubscribe::BindingMapping
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingMapping"); };

	struct BindingNoticed : public InheritedMessage<BindingNoticed,
        PublishSubscribe::BindingNoticed
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingNoticed"); };

	struct BindingRemoved : public InheritedMessage<BindingRemoved,
        PublishSubscribe::BindingRemoved
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingRemoved"); };

	struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        PublishSubscribe::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingRemovedAck"); };

	struct BindingEstablished : public InheritedMessage<BindingEstablished,
        PublishSubscribe::BindingEstablished
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingEstablished"); };

	struct BindingReleased : public InheritedMessage<BindingReleased,
        PublishSubscribe::BindingReleased
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        PublishSubscribe::Subscribe
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        PublishSubscribe::Unsubscribe
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        PublishSubscribe::Data
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::Data"); };

    struct CachedData : public Message<CachedData,
        fields::SubscriptionId,
        fields::Data
    > { CHIRP_MESSAGE_NAME("CachedPublishSubscribe::CachedData"); };
}; // struct CachedPublishSubscribe

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_CACHEDPUBLISHSUBSCRIBE_HPP
