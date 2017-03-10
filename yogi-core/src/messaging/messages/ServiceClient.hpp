#ifndef CHIRP_MESSAGING_MESSAGES_SERVICECLIENT_HPP
#define CHIRP_MESSAGING_MESSAGES_SERVICECLIENT_HPP

#include "../../config.h"
#include "ScatterGather.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct ServiceClient {
    struct TerminalDescription : public InheritedMessage<TerminalDescription,
        ScatterGather::TerminalDescription
    > { CHIRP_MESSAGE_NAME("ServiceClient::TerminalDescription"); };

    struct TerminalMapping : public InheritedMessage<TerminalMapping,
        ScatterGather::TerminalMapping
    > { CHIRP_MESSAGE_NAME("ServiceClient::TerminalMapping"); };

    struct TerminalNoticed : public InheritedMessage<TerminalNoticed,
        ScatterGather::TerminalNoticed
    > { CHIRP_MESSAGE_NAME("ServiceClient::TerminalNoticed"); };

    struct TerminalRemoved : public InheritedMessage<TerminalRemoved,
        ScatterGather::TerminalRemoved
    > { CHIRP_MESSAGE_NAME("ServiceClient::TerminalRemoved"); };

    struct TerminalRemovedAck : public InheritedMessage<TerminalRemovedAck,
        ScatterGather::TerminalRemovedAck
    > { CHIRP_MESSAGE_NAME("ServiceClient::TerminalRemovedAck"); };

    struct BindingDescription : public InheritedMessage<BindingDescription,
        ScatterGather::BindingDescription
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingDescription"); };

    struct BindingMapping : public InheritedMessage<BindingMapping,
        ScatterGather::BindingMapping
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingMapping"); };

    struct BindingNoticed : public InheritedMessage<BindingNoticed,
        ScatterGather::BindingNoticed
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingNoticed"); };

    struct BindingRemoved : public InheritedMessage<BindingRemoved,
        ScatterGather::BindingRemoved
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingRemoved"); };

    struct BindingRemovedAck : public InheritedMessage<BindingRemovedAck,
        ScatterGather::BindingRemovedAck
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingRemovedAck"); };

    struct BindingEstablished : public InheritedMessage<BindingEstablished,
        ScatterGather::BindingEstablished
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingEstablished"); };

    struct BindingReleased : public InheritedMessage<BindingReleased,
        ScatterGather::BindingReleased
    > { CHIRP_MESSAGE_NAME("ServiceClient::BindingReleased"); };

    struct Subscribe : public InheritedMessage<Subscribe,
        ScatterGather::Subscribe
    > { CHIRP_MESSAGE_NAME("ServiceClient::Subscribe"); };

    struct Unsubscribe : public InheritedMessage<Unsubscribe,
        ScatterGather::Unsubscribe
    > { CHIRP_MESSAGE_NAME("ServiceClient::Unsubscribe"); };

    struct Scatter : public InheritedMessage<Scatter,
        ScatterGather::Scatter
    > { CHIRP_MESSAGE_NAME("ServiceClient::Scatter"); };

    struct Gather : public InheritedMessage<Gather,
        ScatterGather::Gather
    > { CHIRP_MESSAGE_NAME("ServiceClient::Gather"); };
}; // struct ServiceClient

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_SERVICECLIENT_HPP
