#include "../../src/messaging/MessageRegister.hpp"
using namespace chirp::messaging;
using namespace chirp::interfaces;
using namespace chirp::base;
using namespace chirp::core;

#include <gmock/gmock.h>


struct MessagingTest : public testing::Test
{
};

TEST_F(MessagingTest, Message)
{
	const auto identifier = Identifier{77u, "Hello", false};
	const auto id         = Id{123u};

	messages::DeafMute::BindingDescription msg1;
	msg1[fields::identifier] = identifier;
	msg1[fields::id]         = id;
	EXPECT_EQ(identifier, msg1[fields::identifier]);
	EXPECT_EQ(id,         msg1[fields::id]);
	EXPECT_TRUE(msg1.type_id().valid());

	auto msg2 = msg1.clone();
	EXPECT_EQ(msg1.type_id(), msg2->type_id());

	IMessage::buffer_type buffer;
	msg2->serialize(buffer);
	messages::DeafMute::BindingDescription msg3;
	msg3.deserialize(buffer, buffer.begin());
	EXPECT_EQ(identifier, msg3[fields::identifier]);
	EXPECT_EQ(id,         msg3[fields::id]);
}

TEST_F(MessagingTest, CreateMessage)
{
	const auto identifier = Identifier{77u, "Hello", false};
	const auto id         = Id{123u};

	auto msg = messages::ScatterGather::BindingDescription::create(
		identifier, id);

	EXPECT_EQ(messages::ScatterGather::BindingDescription{}.type_id(),
		msg.type_id());
	EXPECT_EQ(identifier, msg[fields::identifier]);
	EXPECT_EQ(id,         msg[fields::id]);
}

TEST_F(MessagingTest, CloneMessage)
{
	const auto identifier = Identifier{77u, "Hello", false};
	const auto id         = Id{123u};

	auto msg = messages::ScatterGather::BindingDescription::create(
		identifier, id);
	auto msg2_ = msg.clone();
	auto& msg2 = *dynamic_cast<messages::ScatterGather::BindingDescription*>(
		msg2_.get());

	EXPECT_EQ(msg.type_id(),           msg2.type_id());
	EXPECT_EQ(msg[fields::identifier], msg2[fields::identifier]);
	EXPECT_EQ(msg[fields::id],         msg2[fields::id]);
}

TEST_F(MessagingTest, MessageTypeId)
{
	messages::DeafMute::BindingDescription      msg1;
	messages::PublishSubscribe::Data	        msg2;
	messages::ScatterGather::BindingDescription	msg3;
	EXPECT_NE(msg1.type_id(), msg2.type_id());
	EXPECT_NE(msg1.type_id(), msg3.type_id());
}

TEST_F(MessagingTest, MessageToString)
{
	messages::DeafMute::BindingDescription msg1;
	msg1[fields::identifier] = Identifier{77u, "Hello", false};
	msg1[fields::id]         = Id{123u};

	const char str1[] = "DeafMute::BindingDescription(identifier=(77, Hello), "
		"id=123)";
	EXPECT_EQ(str1, msg1.to_string());

	messages::PublishSubscribe::Data msg2;
	msg2[fields::subscriptionId] = Id{5u};
	msg2[fields::data]           = Buffer("abc", 3);

	const char str2[] = "PublishSubscribe::Data(subscriptionId=5, "
		"data=[61 62 63])";
	EXPECT_EQ(str2, msg2.to_string());

	messages::ScatterGather::Gather msg3;
	msg3[fields::operationId] = Id{80u};
	msg3[fields::data]        = Buffer("abc", 3);
	msg3[fields::gatherFlags] = scatter_gather::GATHER_CONNECTIONLOST
		| scatter_gather::GATHER_FINISHED;

	const char str3[] = "ScatterGather::Gather("
		"operationId=80, gatherFlags=FINISHED | CONNECTIONLOST, "
		"data=[61 62 63])";
	EXPECT_EQ(str3, msg3.to_string());
}
