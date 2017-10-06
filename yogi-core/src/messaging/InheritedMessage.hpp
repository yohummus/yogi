#ifndef YOGI_MESSAGING_INHERITEDMESSAGE_HPP
#define YOGI_MESSAGING_INHERITEDMESSAGE_HPP

#include "../config.h"
#include "Message.hpp"


namespace yogi {
namespace messaging {

template <typename TFinalMessage, typename TInheritedMessage>
class InheritedMessage
	: public TInheritedMessage
{
public:
	template <typename... TValues>
	static TFinalMessage create(TValues... values)
	{
		return TInheritedMessage::template create_impl<TFinalMessage>(values...);
	}

	// implementation in MessageRegister.hpp
	virtual interfaces::IMessage::id_type type_id() const override;

	virtual interfaces::message_ptr clone() const override
	{
		YOGI_ASSERT(dynamic_cast<const TFinalMessage*>(this) != nullptr);
		return std::make_unique<TFinalMessage>(
			*static_cast<const TFinalMessage*>(this));
	}
};

} // namespace messaging
} // namespace yogi

#endif // YOGI_MESSAGING_INHERITEDMESSAGE_HPP
