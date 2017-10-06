#ifndef YOGI_MESSAGING_MESSAGES_DEAFMUTE_HPP
#define YOGI_MESSAGING_MESSAGES_DEAFMUTE_HPP

#include "../../config.h"
#include "../fields/fields.hpp"
#include "../Message.hpp"


namespace yogi {
namespace messaging {
namespace messages {

struct DeafMute {
	struct TerminalDescription : public Message<TerminalDescription,
		fields::Identifier,
		fields::Id
    > { YOGI_MESSAGE_NAME("DeafMute::TerminalDescription"); };

	struct TerminalMapping : public Message<TerminalMapping,
		fields::TerminalId,
        fields::MappedId
    > { YOGI_MESSAGE_NAME("DeafMute::TerminalMapping"); };

	struct TerminalNoticed : public Message<TerminalNoticed,
        fields::TerminalId
    > { YOGI_MESSAGE_NAME("DeafMute::TerminalNoticed"); };

	struct TerminalRemoved : public Message<TerminalRemoved,
        fields::MappedId
    > { YOGI_MESSAGE_NAME("DeafMute::TerminalRemoved"); };

	struct TerminalRemovedAck : public Message<TerminalRemovedAck,
        fields::TerminalId
    > { YOGI_MESSAGE_NAME("DeafMute::TerminalRemovedAck"); };

	struct BindingDescription : public Message<BindingDescription,
		fields::Identifier,
        fields::Id
    > { YOGI_MESSAGE_NAME("DeafMute::BindingDescription"); };

	struct BindingMapping : public Message<BindingMapping,
		fields::BindingId,
        fields::MappedId
    > { YOGI_MESSAGE_NAME("DeafMute::BindingMapping"); };

	struct BindingNoticed : public Message<BindingNoticed,
        fields::BindingId
    > { YOGI_MESSAGE_NAME("DeafMute::BindingNoticed"); };

	struct BindingRemoved : public Message<BindingRemoved,
        fields::MappedId
    > { YOGI_MESSAGE_NAME("DeafMute::BindingRemoved"); };

	struct BindingRemovedAck : public Message<BindingRemovedAck,
        fields::BindingId
    > { YOGI_MESSAGE_NAME("DeafMute::BindingRemovedAck"); };

	struct BindingEstablished : public Message<BindingEstablished,
        fields::BindingId
    > { YOGI_MESSAGE_NAME("DeafMute::BindingEstablished"); };

	struct BindingReleased : public Message<BindingReleased,
        fields::BindingId
    > { YOGI_MESSAGE_NAME("DeafMute::BindingReleased"); };
}; // struct DeafMute

} // namespace messages
} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_MESSAGES_DEAFMUTE_HPP
