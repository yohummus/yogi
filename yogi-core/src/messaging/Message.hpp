#ifndef CHIRP_MESSAGING_MESSAGE_HPP
#define CHIRP_MESSAGING_MESSAGE_HPP

#include "../config.h"
#include "../interfaces/IMessage.hpp"
#include "../serialization/serialize.hpp"
#include "../serialization/deserialize.hpp"

#include <sstream>

#define CHIRP_MESSAGE_NAME(str)                                                \
    virtual const char* name() const override                                  \
    {                                                                          \
        return str;                                                            \
    }


namespace chirp {
namespace messaging {
namespace internal_ {

template <typename TField>
class FieldMember
{
	template <typename TFinalMessage, typename... TFields>
	friend class Message;

	typedef TField                field_type;
	typedef typename TField::type value_type;

private:
	value_type m_value;

protected:
	const value_type& value() const
	{
		return m_value;
	}

	value_type& value()
	{
		return m_value;
	}

	std::string to_string() const
	{
		std::stringstream ss;
		ss << field_type::name();
		ss << "=";
		ss << m_value;
		return ss.str();
	}

public:
	const value_type& operator[] (field_type) const
	{
		return m_value;
	}

	value_type& operator[] (field_type)
	{
		return m_value;
	}
};

} // namespace internal_

template <typename TFinalMessage, typename... TFields>
class Message
	: public interfaces::IMessage
	, public internal_::FieldMember<TFields>...
{
	struct dummy_t {};

protected:
	template <typename TField, typename TFinalMessage_, typename TValue>
	static dummy_t set_field_value(TFinalMessage_& msg, TValue value)
	{
		msg.internal_::template FieldMember<TField>::value() = value;
		return dummy_t{};
	}

    template <typename TFinalMessage_>
    static TFinalMessage_ create_impl(typename TFields::type... values)
    {
        TFinalMessage_ msg;
		dummy_t dummy[] = { set_field_value<TFields>(msg, values)... };
        return msg;
    }

public:
	template <typename TFinalMessage_ = TFinalMessage>
	static TFinalMessage_ create(typename TFields::type... values)
	{
		return create_impl<TFinalMessage_>(values...);
	}

	// implementation in MessageRegister.hpp
    virtual interfaces::IMessage::id_type type_id() const override;

	virtual std::string to_string() const override
    {
		auto fields = std::vector<std::string>{
			internal_::FieldMember<TFields>::to_string()...
		};

		std::stringstream ss;
        ss << name() << "(";
		for (std::size_t i = 0; i < fields.size(); ++i) {
			ss << fields[i];
			if (i < fields.size() - 1) {
				ss << ", ";
			}
		}
		ss << ")";

		return ss.str();
	}

	virtual interfaces::message_ptr clone() const override
	{
		CHIRP_ASSERT(dynamic_cast<const TFinalMessage*>(this) != nullptr);
		return std::make_unique<TFinalMessage>(
			*static_cast<const TFinalMessage*>(this));
	}

	virtual void serialize(buffer_type& buffer) const override
	{
		serialization::serialize(buffer,
			internal_::FieldMember<TFields>::value()...);
	}

	virtual void deserialize(const buffer_type& buffer,
		buffer_type::const_iterator start) override
	{
		serialization::deserialize(buffer, start,
			internal_::FieldMember<TFields>::value()...);
	}

	template <typename TField>
	const typename internal_::FieldMember<TField>::value_type& operator[] (TField field_type) const
	{
		return internal_::FieldMember<TField>::operator[] (field_type);
	}

	template <typename TField>
	typename internal_::FieldMember<TField>::value_type& operator[] (TField field_type)
	{
		return internal_::FieldMember<TField>::operator[] (field_type);
	}
};

} // namespace messaging
} // namespace chirp

#endif // CHIRP_MESSAGING_MESSAGE_HPP
