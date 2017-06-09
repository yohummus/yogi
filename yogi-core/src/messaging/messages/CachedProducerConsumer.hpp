#ifndef YOGI_MESSAGING_MESSAGES_CACHEDPRODUCERCONSUMER_HPP
#define YOGI_MESSAGING_MESSAGES_CACHEDPRODUCERCONSUMER_HPP

#include "../../config.h"
#include "CachedPublishSubscribe.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct CachedProducerConsumer {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        CachedPublishSubscribe::TerminalDescription
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        CachedPublishSubscribe::TerminalMapping
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        CachedPublishSubscribe::TerminalNoticed
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        CachedPublishSubscribe::TerminalRemoved
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        CachedPublishSubscribe::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        CachedPublishSubscribe::BindingDescription
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        CachedPublishSubscribe::BindingMapping
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        CachedPublishSubscribe::BindingNoticed
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        CachedPublishSubscribe::BindingRemoved
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        CachedPublishSubscribe::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        CachedPublishSubscribe::BindingEstablished
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        CachedPublishSubscribe::BindingReleased
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        CachedPublishSubscribe::Subscribe
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        CachedPublishSubscribe::Unsubscribe
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        CachedPublishSubscribe::Data
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::Data"); };

    struct CachedData : public InheritedMessage<CachedData,
        CachedPublishSubscribe::CachedData
    > { YOGI_MESSAGE_NAME("CachedProducerConsumer::CachedData"); };
}; // struct CachedProducerConsumer

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_CACHEDPRODUCERCONSUMER_HPP
