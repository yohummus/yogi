#ifndef YOGI_MESSAGING_MESSAGES_MASTERSLAVE_HPP
#define YOGI_MESSAGING_MESSAGES_MASTERSLAVE_HPP

#include "../../config.h"
#include "PublishSubscribe.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct MasterSlave {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        PublishSubscribe::TerminalDescription
    > { YOGI_MESSAGE_NAME("MasterSlave::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        PublishSubscribe::TerminalMapping
    > { YOGI_MESSAGE_NAME("MasterSlave::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        PublishSubscribe::TerminalNoticed
    > { YOGI_MESSAGE_NAME("MasterSlave::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        PublishSubscribe::TerminalRemoved
    > { YOGI_MESSAGE_NAME("MasterSlave::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        PublishSubscribe::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("MasterSlave::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        PublishSubscribe::BindingDescription
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        PublishSubscribe::BindingMapping
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        PublishSubscribe::BindingNoticed
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        PublishSubscribe::BindingRemoved
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        PublishSubscribe::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        PublishSubscribe::BindingEstablished
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        PublishSubscribe::BindingReleased
    > { YOGI_MESSAGE_NAME("MasterSlave::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        PublishSubscribe::Subscribe
    > { YOGI_MESSAGE_NAME("MasterSlave::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        PublishSubscribe::Unsubscribe
    > { YOGI_MESSAGE_NAME("MasterSlave::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        PublishSubscribe::Data
    > { YOGI_MESSAGE_NAME("MasterSlave::Data"); };
}; // struct MasterSlave

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_MASTERSLAVE_HPP
