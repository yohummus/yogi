#ifndef CHIRP_MESSAGING_MESSAGEREGISTER_HPP
#define CHIRP_MESSAGING_MESSAGEREGISTER_HPP

#include "../config.h"
#include "../interfaces/ICommunicator.hpp"
#include "../interfaces/IConnection.hpp"
#include "../interfaces/IMessage.hpp"
#include "../serialization/deserialize.hpp"
#include "messages/CachedMasterSlave.hpp"
#include "messages/CachedProducerConsumer.hpp"
#include "messages/CachedPublishSubscribe.hpp"
#include "messages/DeafMute.hpp"
#include "messages/MasterSlave.hpp"
#include "messages/ProducerConsumer.hpp"
#include "messages/PublishSubscribe.hpp"
#include "messages/ScatterGather.hpp"
#include "messages/ServiceClient.hpp"

#include <array>


namespace chirp {
namespace messaging {
namespace internal_ {

template <typename...>
struct IndexOf;

template <typename T, typename First, typename... Remaining>
struct IndexOf<T, First, Remaining...>
{
	enum { value = IndexOf<T, Remaining...>::value + 1 };
};

template <typename T, typename... Remaining>
struct IndexOf<T, T, Remaining...>
{
	enum { value = 0 };
};

template <typename TMessage, typename... TAllMessages>
struct MessageRegisterMember
{
	static void deserialize_and_forward_message(
		const interfaces::IMessage::buffer_type& buffer,
		interfaces::IMessage::buffer_type::const_iterator start,
		interfaces::ICommunicator& communicator,
		interfaces::IConnection& origin)
	{
		TMessage msg;
		msg.deserialize(buffer, start);
		communicator.on_message_received(std::move(msg), origin);
	}
};

template <typename... TMessages>
class MessageRegisterImpl
	: public MessageRegisterMember<TMessages, TMessages...>...
{
	typedef void (*deserialize_and_forward_message_fn) (
		const interfaces::IMessage::buffer_type&,
		interfaces::IMessage::buffer_type::const_iterator,
		interfaces::ICommunicator& communicator,
		interfaces::IConnection& origin);
	typedef std::array<deserialize_and_forward_message_fn, sizeof...(TMessages)>
		deserialize_and_forward_message_lut_type;

public:
	template <typename TMessage>
	static base::Id message_type_id()
	{
		return base::Id{IndexOf<TMessage, TMessages...>::value + 1};
	}

	static void deserialize_and_forward_message(
		interfaces::IMessage::id_type msgTypeId,
		const interfaces::IMessage::buffer_type& buffer,
		interfaces::IMessage::buffer_type::const_iterator start,
		interfaces::ICommunicator& communicator,
		interfaces::IConnection& origin)
	{
		static const deserialize_and_forward_message_lut_type lut{{
            MessageRegisterMember<TMessages, TMessages...>
                ::deserialize_and_forward_message...
        }};

		CHIRP_ASSERT(msgTypeId.valid());
		CHIRP_ASSERT(msgTypeId.number() <= lut.size());
		lut[msgTypeId.number() - 1](buffer, start, communicator, origin);
	}
};

} // namespace internal_

class MessageRegister
	: public internal_::MessageRegisterImpl<
		messages::DeafMute::TerminalDescription,
		messages::DeafMute::TerminalMapping,
		messages::DeafMute::TerminalNoticed,
		messages::DeafMute::TerminalRemoved,
		messages::DeafMute::TerminalRemovedAck,
		messages::DeafMute::BindingDescription,
		messages::DeafMute::BindingMapping,
		messages::DeafMute::BindingNoticed,
		messages::DeafMute::BindingRemoved,
		messages::DeafMute::BindingRemovedAck,
		messages::DeafMute::BindingEstablished,
		messages::DeafMute::BindingReleased,

		messages::PublishSubscribe::TerminalDescription,
		messages::PublishSubscribe::TerminalMapping,
		messages::PublishSubscribe::TerminalNoticed,
		messages::PublishSubscribe::TerminalRemoved,
		messages::PublishSubscribe::TerminalRemovedAck,
		messages::PublishSubscribe::BindingDescription,
		messages::PublishSubscribe::BindingMapping,
		messages::PublishSubscribe::BindingNoticed,
		messages::PublishSubscribe::BindingRemoved,
		messages::PublishSubscribe::BindingRemovedAck,
		messages::PublishSubscribe::BindingEstablished,
		messages::PublishSubscribe::BindingReleased,
		messages::PublishSubscribe::Subscribe,
		messages::PublishSubscribe::Unsubscribe,
		messages::PublishSubscribe::Data,

		messages::ScatterGather::TerminalDescription,
		messages::ScatterGather::TerminalMapping,
		messages::ScatterGather::TerminalNoticed,
		messages::ScatterGather::TerminalRemoved,
		messages::ScatterGather::TerminalRemovedAck,
		messages::ScatterGather::BindingDescription,
		messages::ScatterGather::BindingMapping,
		messages::ScatterGather::BindingNoticed,
		messages::ScatterGather::BindingRemoved,
		messages::ScatterGather::BindingRemovedAck,
		messages::ScatterGather::BindingEstablished,
		messages::ScatterGather::BindingReleased,
		messages::ScatterGather::Subscribe,
		messages::ScatterGather::Unsubscribe,
		messages::ScatterGather::Scatter,
		messages::ScatterGather::Gather,

        messages::CachedPublishSubscribe::TerminalDescription,
        messages::CachedPublishSubscribe::TerminalMapping,
        messages::CachedPublishSubscribe::TerminalNoticed,
        messages::CachedPublishSubscribe::TerminalRemoved,
        messages::CachedPublishSubscribe::TerminalRemovedAck,
        messages::CachedPublishSubscribe::BindingDescription,
        messages::CachedPublishSubscribe::BindingMapping,
        messages::CachedPublishSubscribe::BindingNoticed,
        messages::CachedPublishSubscribe::BindingRemoved,
        messages::CachedPublishSubscribe::BindingRemovedAck,
        messages::CachedPublishSubscribe::BindingEstablished,
        messages::CachedPublishSubscribe::BindingReleased,
        messages::CachedPublishSubscribe::Subscribe,
        messages::CachedPublishSubscribe::Unsubscribe,
        messages::CachedPublishSubscribe::Data,
        messages::CachedPublishSubscribe::CachedData,

        messages::ProducerConsumer::TerminalDescription,
        messages::ProducerConsumer::TerminalMapping,
        messages::ProducerConsumer::TerminalNoticed,
        messages::ProducerConsumer::TerminalRemoved,
        messages::ProducerConsumer::TerminalRemovedAck,
        messages::ProducerConsumer::BindingDescription,
        messages::ProducerConsumer::BindingMapping,
        messages::ProducerConsumer::BindingNoticed,
        messages::ProducerConsumer::BindingRemoved,
        messages::ProducerConsumer::BindingRemovedAck,
        messages::ProducerConsumer::BindingEstablished,
        messages::ProducerConsumer::BindingReleased,
        messages::ProducerConsumer::Subscribe,
        messages::ProducerConsumer::Unsubscribe,
        messages::ProducerConsumer::Data,

        messages::CachedProducerConsumer::TerminalDescription,
        messages::CachedProducerConsumer::TerminalMapping,
        messages::CachedProducerConsumer::TerminalNoticed,
        messages::CachedProducerConsumer::TerminalRemoved,
        messages::CachedProducerConsumer::TerminalRemovedAck,
        messages::CachedProducerConsumer::BindingDescription,
        messages::CachedProducerConsumer::BindingMapping,
        messages::CachedProducerConsumer::BindingNoticed,
        messages::CachedProducerConsumer::BindingRemoved,
        messages::CachedProducerConsumer::BindingRemovedAck,
        messages::CachedProducerConsumer::BindingEstablished,
        messages::CachedProducerConsumer::BindingReleased,
        messages::CachedProducerConsumer::Subscribe,
        messages::CachedProducerConsumer::Unsubscribe,
        messages::CachedProducerConsumer::Data,
        messages::CachedProducerConsumer::CachedData,

        messages::MasterSlave::TerminalDescription,
        messages::MasterSlave::TerminalMapping,
        messages::MasterSlave::TerminalNoticed,
        messages::MasterSlave::TerminalRemoved,
        messages::MasterSlave::TerminalRemovedAck,
        messages::MasterSlave::BindingDescription,
        messages::MasterSlave::BindingMapping,
        messages::MasterSlave::BindingNoticed,
        messages::MasterSlave::BindingRemoved,
        messages::MasterSlave::BindingRemovedAck,
        messages::MasterSlave::BindingEstablished,
        messages::MasterSlave::BindingReleased,
        messages::MasterSlave::Subscribe,
        messages::MasterSlave::Unsubscribe,
        messages::MasterSlave::Data,

        messages::CachedMasterSlave::TerminalDescription,
        messages::CachedMasterSlave::TerminalMapping,
        messages::CachedMasterSlave::TerminalNoticed,
        messages::CachedMasterSlave::TerminalRemoved,
        messages::CachedMasterSlave::TerminalRemovedAck,
        messages::CachedMasterSlave::BindingDescription,
        messages::CachedMasterSlave::BindingMapping,
        messages::CachedMasterSlave::BindingNoticed,
        messages::CachedMasterSlave::BindingRemoved,
        messages::CachedMasterSlave::BindingRemovedAck,
        messages::CachedMasterSlave::BindingEstablished,
        messages::CachedMasterSlave::BindingReleased,
        messages::CachedMasterSlave::Subscribe,
        messages::CachedMasterSlave::Unsubscribe,
        messages::CachedMasterSlave::Data,
        messages::CachedMasterSlave::CachedData,

        messages::ServiceClient::TerminalDescription,
		messages::ServiceClient::TerminalMapping,
		messages::ServiceClient::TerminalNoticed,
		messages::ServiceClient::TerminalRemoved,
		messages::ServiceClient::TerminalRemovedAck,
		messages::ServiceClient::BindingDescription,
		messages::ServiceClient::BindingMapping,
		messages::ServiceClient::BindingNoticed,
		messages::ServiceClient::BindingRemoved,
		messages::ServiceClient::BindingRemovedAck,
		messages::ServiceClient::BindingEstablished,
		messages::ServiceClient::BindingReleased,
		messages::ServiceClient::Subscribe,
		messages::ServiceClient::Unsubscribe,
		messages::ServiceClient::Scatter,
		messages::ServiceClient::Gather
	>
{
};

template <typename TFinalMessage, typename... TFields>
interfaces::IMessage::id_type
Message<TFinalMessage, TFields...>::type_id() const
{
	return MessageRegister::message_type_id<TFinalMessage>();
}

template <typename TFinalMessage, typename TInheritedMessage>
interfaces::IMessage::id_type
InheritedMessage<TFinalMessage, TInheritedMessage>::type_id() const
{
	return MessageRegister::message_type_id<TFinalMessage>();
}

} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGEREGISTER_HPP
