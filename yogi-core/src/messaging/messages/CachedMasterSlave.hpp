#ifndef YOGI_MESSAGING_MESSAGES_CACHEDMASTERSLAVE_HPP
#define YOGI_MESSAGING_MESSAGES_CACHEDMASTERSLAVE_HPP

#include "../../config.h"
#include "CachedPublishSubscribe.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct CachedMasterSlave {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        CachedPublishSubscribe::TerminalDescription
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        CachedPublishSubscribe::TerminalMapping
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        CachedPublishSubscribe::TerminalNoticed
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        CachedPublishSubscribe::TerminalRemoved
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        CachedPublishSubscribe::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        CachedPublishSubscribe::BindingDescription
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        CachedPublishSubscribe::BindingMapping
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        CachedPublishSubscribe::BindingNoticed
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        CachedPublishSubscribe::BindingRemoved
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        CachedPublishSubscribe::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        CachedPublishSubscribe::BindingEstablished
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        CachedPublishSubscribe::BindingReleased
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        CachedPublishSubscribe::Subscribe
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        CachedPublishSubscribe::Unsubscribe
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::Unsubscribe"); };

    struct Data : public InheritedMessage<Data,
        CachedPublishSubscribe::Data
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::Data"); };

    struct CachedData : public InheritedMessage<CachedData,
        CachedPublishSubscribe::CachedData
    > { YOGI_MESSAGE_NAME("CachedMasterSlave::CachedData"); };
}; // struct CachedMasterSlave

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_CACHEDMASTERSLAVE_HPP
