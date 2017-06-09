#ifndef YOGI_MESSAGING_MESSAGES_SERVICECLIENT_HPP
#define YOGI_MESSAGING_MESSAGES_SERVICECLIENT_HPP

#include "../../config.h"
#include "ScatterGather.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct ServiceClient {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        ScatterGather::TerminalDescription
    > { YOGI_MESSAGE_NAME("ServiceClient::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        ScatterGather::TerminalMapping
    > { YOGI_MESSAGE_NAME("ServiceClient::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        ScatterGather::TerminalNoticed
    > { YOGI_MESSAGE_NAME("ServiceClient::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        ScatterGather::TerminalRemoved
    > { YOGI_MESSAGE_NAME("ServiceClient::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        ScatterGather::TerminalRemovedAck
    > { YOGI_MESSAGE_NAME("ServiceClient::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        ScatterGather::BindingDescription
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        ScatterGather::BindingMapping
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        ScatterGather::BindingNoticed
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        ScatterGather::BindingRemoved
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        ScatterGather::BindingRemovedAck
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        ScatterGather::BindingEstablished
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        ScatterGather::BindingReleased
    > { YOGI_MESSAGE_NAME("ServiceClient::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        ScatterGather::Subscribe
    > { YOGI_MESSAGE_NAME("ServiceClient::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        ScatterGather::Unsubscribe
    > { YOGI_MESSAGE_NAME("ServiceClient::Unsubscribe"); };

    struct Scatter : public InheritedMessage<Scatter,
        ScatterGather::Scatter
    > { YOGI_MESSAGE_NAME("ServiceClient::Scatter"); };

    struct Gather : public InheritedMessage<Gather,
        ScatterGather::Gather
    > { YOGI_MESSAGE_NAME("ServiceClient::Gather"); };
}; // struct ServiceClient

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_SERVICECLIENT_HPP
