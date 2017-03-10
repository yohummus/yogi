#ifndef CHIRP_MESSAGING_MESSAGES_CACHEDPRODUCERCONSUMER_HPP
#define CHIRP_MESSAGING_MESSAGES_CACHEDPRODUCERCONSUMER_HPP

#include "../../config.h"
#include "CachedPublishSubscribe.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct CachedProducerConsumer {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        CachedPublishSubscribe::TerminalDescription
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        CachedPublishSubscribe::TerminalMapping
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        CachedPublishSubscribe::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        CachedPublishSubscribe::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        CachedPublishSubscribe::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        CachedPublishSubscribe::BindingDescription
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        CachedPublishSubscribe::BindingMapping
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        CachedPublishSubscribe::BindingNoticed
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        CachedPublishSubscribe::BindingRemoved
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        CachedPublishSubscribe::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        CachedPublishSubscribe::BindingEstablished
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        CachedPublishSubscribe::BindingReleased
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        CachedPublishSubscribe::Subscribe
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        CachedPublishSubscribe::Unsubscribe
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        CachedPublishSubscribe::Data
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::Data"); };

    struct CachedData : public InheritedMessage<CachedData,
        CachedPublishSubscribe::CachedData
    > { CHIRP_MESSAGE_NAME("CachedProducerConsumer::CachedData"); };
}; // struct CachedProducerConsumer

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_CACHEDPRODUCERCONSUMER_HPP
