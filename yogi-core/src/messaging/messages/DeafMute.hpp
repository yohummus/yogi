#ifndef CHIRP_MESSAGING_MESSAGES_DEAFMUTE_HPP
#define CHIRP_MESSAGING_MESSAGES_DEAFMUTE_HPP

#include "../../config.h"
#include "../fields/fields.hpp"
#include "../Message.hpp"


namespace chirp {
namespace messaging {
namespace messages {

struct DeafMute {
	struct TerminalDescription : public Message<TerminalDescription,
		fields::Identifier,
		fields::Id
    > { CHIRP_MESSAGE_NAME("DeafMute::TerminalDescription"); };

	struct TerminalMapping : public Message<TerminalMapping,
		fields::TerminalId,
        fields::MappedId
    > { CHIRP_MESSAGE_NAME("DeafMute::TerminalMapping"); };

	struct TerminalNoticed : public Message<TerminalNoticed,
        fields::TerminalId
    > { CHIRP_MESSAGE_NAME("DeafMute::TerminalNoticed"); };

	struct TerminalRemoved : public Message<TerminalRemoved,
        fields::MappedId
    > { CHIRP_MESSAGE_NAME("DeafMute::TerminalRemoved"); };

	struct TerminalRemovedAck : public Message<TerminalRemovedAck,
        fields::TerminalId
    > { CHIRP_MESSAGE_NAME("DeafMute::TerminalRemovedAck"); };

	struct BindingDescription : public Message<BindingDescription,
		fields::Identifier,
        fields::Id
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingDescription"); };

	struct BindingMapping : public Message<BindingMapping,
		fields::BindingId,
        fields::MappedId
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingMapping"); };

	struct BindingNoticed : public Message<BindingNoticed,
        fields::BindingId
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingNoticed"); };

	struct BindingRemoved : public Message<BindingRemoved,
        fields::MappedId
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingRemoved"); };

	struct BindingRemovedAck : public Message<BindingRemovedAck,
        fields::BindingId
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingRemovedAck"); };

	struct BindingEstablished : public Message<BindingEstablished,
        fields::BindingId
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingEstablished"); };

	struct BindingReleased : public Message<BindingReleased,
        fields::BindingId
    > { CHIRP_MESSAGE_NAME("DeafMute::BindingReleased"); };
}; // struct DeafMute

} // namespace messages
} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGES_DEAFMUTE_HPP
