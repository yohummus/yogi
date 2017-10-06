#ifndef YOGI_MESSAGING_MESSAGES_PRODUCERCONSUMER_HPP
#define YOGI_MESSAGING_MESSAGES_PRODUCERCONSUMER_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct ProducerConsumer {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        PublishSubscribe::TerminalDescription
    > { YOGI_MESSAGE_NAME("ProducerConsumer::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        PublishSubscribe::TerminalMapping
    > { YOGI_MESSAGE_NAME("ProducerConsumer::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        PublishSubscribe::TerminalNoticed
    > { YOGI_MESSAGE_NAME("ProducerConsumer::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        PublishSubscribe::TerminalRemoved
    > { YOGI_MESSAGE_NAME("ProducerConsumer::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        PublishSubscribe::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("ProducerConsumer::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        PublishSubscribe::BindingDescription
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        PublishSubscribe::BindingMapping
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        PublishSubscribe::BindingNoticed
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        PublishSubscribe::BindingRemoved
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        PublishSubscribe::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        PublishSubscribe::BindingEstablished
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        PublishSubscribe::BindingReleased
    > { YOGI_MESSAGE_NAME("ProducerConsumer::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        PublishSubscribe::Subscribe
    > { YOGI_MESSAGE_NAME("ProducerConsumer::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        PublishSubscribe::Unsubscribe
    > { YOGI_MESSAGE_NAME("ProducerConsumer::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        PublishSubscribe::Data
    > { YOGI_MESSAGE_NAME("ProducerConsumer::Data"); };
}; // struct ProducerConsumer

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_PRODUCERCONSUMER_HPP
