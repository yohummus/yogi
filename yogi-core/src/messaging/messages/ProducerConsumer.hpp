#ifndef CHIRP_MESSAGING_MESSAGES_PRODUCERCONSUMER_HPP
#define CHIRP_MESSAGING_MESSAGES_PRODUCERCONSUMER_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct ProducerConsumer {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        PublishSubscribe::TerminalDescription
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        PublishSubscribe::TerminalMapping
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        PublishSubscribe::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        PublishSubscribe::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        PublishSubscribe::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        PublishSubscribe::BindingDescription
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        PublishSubscribe::BindingMapping
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        PublishSubscribe::BindingNoticed
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        PublishSubscribe::BindingRemoved
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        PublishSubscribe::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        PublishSubscribe::BindingEstablished
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        PublishSubscribe::BindingReleased
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        PublishSubscribe::Subscribe
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        PublishSubscribe::Unsubscribe
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        PublishSubscribe::Data
    > { CHIRP_MESSAGE_NAME("ProducerConsumer::Data"); };
}; // struct ProducerConsumer

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_PRODUCERCONSUMER_HPP
