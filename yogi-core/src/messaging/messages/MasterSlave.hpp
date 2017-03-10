#ifndef CHIRP_MESSAGING_MESSAGES_MASTERSLAVE_HPP
#define CHIRP_MESSAGING_MESSAGES_MASTERSLAVE_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct MasterSlave {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        PublishSubscribe::TerminalDescription
    > { CHIRP_MESSAGE_NAME("MasterSlave::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        PublishSubscribe::TerminalMapping
    > { CHIRP_MESSAGE_NAME("MasterSlave::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        PublishSubscribe::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("MasterSlave::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        PublishSubscribe::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("MasterSlave::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        PublishSubscribe::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("MasterSlave::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        PublishSubscribe::BindingDescription
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        PublishSubscribe::BindingMapping
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        PublishSubscribe::BindingNoticed
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        PublishSubscribe::BindingRemoved
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        PublishSubscribe::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        PublishSubscribe::BindingEstablished
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        PublishSubscribe::BindingReleased
    > { CHIRP_MESSAGE_NAME("MasterSlave::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        PublishSubscribe::Subscribe
    > { CHIRP_MESSAGE_NAME("MasterSlave::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        PublishSubscribe::Unsubscribe
    > { CHIRP_MESSAGE_NAME("MasterSlave::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        PublishSubscribe::Data
    > { CHIRP_MESSAGE_NAME("MasterSlave::Data"); };
}; // struct MasterSlave

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_MASTERSLAVE_HPP
