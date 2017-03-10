#include "../../src/core/Leaf.hpp"
using namespace chirp::base;
using namespace chirp::core;
using namespace chirp::interfaces;
using namespace chirp::messaging::messages;

#include "../mocks/SchedulerMock.hpp"
#include "../mocks/ConnectionMock.hpp"
#include "../mocks/BindingMock.hpp"
#include "../mocks/DeafMuteTerminalMock.hpp"
#include "../mocks/PublishSubscribeTerminalMock.hpp"
#include "../mocks/ScatterGatherTerminalMock.hpp"
#include "../mocks/CachedPublishSubscribeTerminalMock.hpp"
#include "../mocks/MasterSlaveTerminalMock.hpp"
#include "../mocks/CachedMasterSlaveTerminalMock.hpp"
#include "../mocks/LeafMock.hpp"
#include "../mocks/MessageMock.hpp"
using namespace mocks;


struct LeafTest : public testing::Test
{
    std::shared_ptr<SchedulerMock>                       scheduler;
    std::shared_ptr<testing::StrictMock<ConnectionMock>> connection;
    std::shared_ptr<Leaf>                                uut;

    virtual void SetUp() override
    {
        scheduler  = std::make_shared<SchedulerMock>();
        connection = std::make_shared<testing::StrictMock<ConnectionMock>>();
        uut        = std::make_shared<Leaf>(*scheduler);
    }

    template <typename TFirstMessage, typename... TRemainingMessages>
    struct _ignore_messages
    {
        void operator() (LeafTest& test) const
        {
            _ignore_messages<TFirstMessage>()(test);
            _ignore_messages<TRemainingMessages...>()(test);
        }
    };

    template <typename TFirstMessage>
    struct _ignore_messages<TFirstMessage>
    {
        void operator() (LeafTest& test) const
        {
            EXPECT_CALL(*test.connection, send(MsgType(TFirstMessage{})))
                .Times(AnyNumber());
        }
    };

    template <typename... TMessages>
    void ignore_messages()
    {
        _ignore_messages<TMessages...>()(*this);
    }

    std::shared_ptr<ScatterGatherTerminalMock> make_scatter_gather_terminal(
        Id id, Id mappedId = Id{})
    {
        auto terminal = std::make_shared<ScatterGatherTerminalMock>(*uut,
            Identifier{0u, std::to_string(mappedId.number()), false});

        if (mappedId) {
            uut->on_message_received(ScatterGather::TerminalMapping::create(
                id, mappedId), *connection);
        }
        else {
            uut->on_message_received(ScatterGather::TerminalNoticed::create(
                id), *connection);
        }

        return terminal;
    }
};

TEST_F(LeafTest, NewConnection)
{
    auto tm = std::make_shared<DeafMuteTerminalMock>(*uut,
        Identifier{0u, "T", false});
    auto bd = std::make_shared<BindingMock<deaf_mute::LeafLogic<>>>(*tm, "B");

    for (bool remoteIsNode : {false, true}) {
        EXPECT_CALL(*connection, remote_is_node())
            .WillRepeatedly(Return(remoteIsNode));

        EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalDescription::create(
            Identifier{0u, "T", false}, Id{1}))))
            .Times(remoteIsNode ? 1 : 0);
        EXPECT_CALL(*connection, send(Msg(DeafMute::BindingDescription::create(
            Identifier{0u, "B", false}, Id{1}))));
        uut->on_new_connection(*connection);
        uut->on_connection_started(*connection);

        uut->on_connection_destroyed(*connection);
    }
}

TEST_F(LeafTest, AlreadyConnected)
{
    ASSERT_NO_THROW(uut->on_new_connection(*connection));
    ASSERT_THROW(uut->on_new_connection(*connection),
        api::ExceptionT<CHIRP_ERR_ALREADY_CONNECTED>);
    ASSERT_NO_THROW(uut->on_connection_destroyed(*connection));
}

TEST_F(LeafTest, AmbiguousTerminals)
{
    auto tm = std::make_shared<DeafMuteTerminalMock>(*uut,
        Identifier{1u, "A", false});
    EXPECT_THROW((std::make_shared<mocks::DeafMuteTerminalMock>(*uut,
        Identifier{1u, "A", false})),
        api::ExceptionT<CHIRP_ERR_AMBIGUOUS_IDENTIFIER>);
}

TEST_F(LeafTest, NewTerminal)
{
    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add terminal A
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalDescription::create(
        Identifier{0u, "A", false}, Id{1}))));
    auto t1 = std::make_shared<DeafMuteTerminalMock>(*uut,
        Identifier{0u, "A", false});
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // add terminal B
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalDescription::create(
        Identifier{0u, "B", false}, Id{2}))));
    auto t2 = std::make_shared<DeafMuteTerminalMock>(*uut,
        Identifier{0u, "B", false});
    uut->on_message_received(DeafMute::TerminalNoticed::create(
        Id{2}), *connection);

    // remove terminal A
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalRemoved::create(
        Id{123}))));
    t1.reset();

    // remove terminal B
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalRemoved::create(
        Id{2}))))
        .Times(0);
    t2.reset();
}

TEST_F(LeafTest, NewBinding)
{
    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));
    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add terminal A
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalDescription::create(
        Identifier{0u, "A", false}, Id{1}))));
    auto tA = std::make_shared<DeafMuteTerminalMock>(*uut,
        Identifier{0u, "A", false});
    uut->on_message_received(DeafMute::TerminalMapping::create(Id{1}, Id{123}),
        *connection);

    // add binding b1
    EXPECT_CALL(*connection, send(Msg(DeafMute::BindingDescription::create(
        Identifier{0u, "b1", false}, Id{1}))));
    auto b1 = std::make_shared<BindingMock<deaf_mute::LeafLogic<>>>(*tA, "b1");

    EXPECT_CALL(*b1, publish_state(IBinding::STATE_ESTABLISHED));
    uut->on_message_received(DeafMute::BindingMapping::create(Id{1}, Id{777}),
        *connection);

    // add binding b2
    EXPECT_CALL(*connection, send(Msg(DeafMute::BindingDescription::create(
        Identifier{0u, "b2", false}, Id{2}))));
    auto b2 = std::make_shared<BindingMock<deaf_mute::LeafLogic<>>>(*tA, "b2");
    uut->on_message_received(DeafMute::BindingNoticed::create(
        Id{2}), *connection);

    // remove binding b1
    EXPECT_CALL(*connection, send(Msg(DeafMute::BindingRemoved::create(
        Id{777}))));
    b1.reset();

    // remove binding b2
    b2.reset();

    // remove terminal A
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalRemoved::create(
        Id{123}))));
    tA.reset();
}

TEST_F(LeafTest, EstablishAndReleaseBinding)
{
    // register a node connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(true));
    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add terminal A
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalDescription::create(
        Identifier{0u, "A", false}, Id{1}))));
    auto tA = std::make_shared<DeafMuteTerminalMock>(*uut,
        Identifier{0u, "A", false});
    uut->on_message_received(DeafMute::TerminalMapping::create(Id{1}, Id{123}),
        *connection);

    // add binding b1
    EXPECT_CALL(*connection, send(Msg(DeafMute::BindingDescription::create(
        Identifier{0u, "b1", false}, Id{1}))));
    auto b1 = std::make_shared<BindingMock<deaf_mute::LeafLogic<>>>(*tA, "b1");

    uut->on_message_received(DeafMute::BindingMapping::create(Id{1}, Id{777}),
        *connection);

    // establish the binding
    EXPECT_CALL(*b1, publish_state(IBinding::STATE_ESTABLISHED));
    uut->on_message_received(DeafMute::BindingEstablished::create(Id{1}),
        *connection);

    // release the binding
    EXPECT_CALL(*b1, publish_state(IBinding::STATE_RELEASED));
    uut->on_message_received(DeafMute::BindingReleased::create(Id{1}),
        *connection);

    // remove binding B1
    EXPECT_CALL(*connection, send(Msg(DeafMute::BindingRemoved::create(
        Id{777}))));
    b1.reset();

    // remove terminal T1
    EXPECT_CALL(*connection, send(Msg(DeafMute::TerminalRemoved::create(
        Id{123}))));
    tA.reset();
}

TEST_F(LeafTest, PublishSubscribe)
{
    // we are only interested in publish-subscribe-related messages
    ignore_messages<PublishSubscribe::TerminalDescription,
        PublishSubscribe::TerminalMapping, PublishSubscribe::TerminalNoticed,
        PublishSubscribe::TerminalRemoved, PublishSubscribe::TerminalRemovedAck,
        PublishSubscribe::BindingDescription, PublishSubscribe::BindingMapping,
        PublishSubscribe::BindingNoticed, PublishSubscribe::BindingRemoved,
        PublishSubscribe::BindingRemovedAck>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add publish-subscribe terminal T1 and map it
    auto t1 = std::make_shared<PublishSubscribeTerminalMock>(*uut,
        Identifier{0u, "T1", false});
    uut->on_message_received(PublishSubscribe::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // send a message via T1
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};

    EXPECT_CALL(*connection, send(Msg(PublishSubscribe::Data::create(
        Id{123}, buffer))));
    EXPECT_TRUE(t1->publish(Buffer{buffer}));

    // de-register the leaf connection
    uut->on_connection_destroyed(*connection);

    // register a node connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(true));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);
    uut->on_message_received(PublishSubscribe::TerminalMapping::create(
        Id{1}, Id{456}), *connection);

    // send a message via T1
    EXPECT_CALL(*connection, send(MsgType(PublishSubscribe::Data{})))
        .Times(0);
    EXPECT_FALSE(t1->publish(Buffer{buffer}));

    // subscribe and send a message via T1
    uut->on_message_received(PublishSubscribe::Subscribe::create(
        Id{1}), *connection);

    EXPECT_CALL(*connection, send(Msg(PublishSubscribe::Data::create(
        Id{456}, buffer))));
    EXPECT_TRUE(t1->publish(Buffer{buffer}));

    // unsubscribe and send a message via T1
    uut->on_message_received(PublishSubscribe::Unsubscribe::create(
        Id{1}), *connection);

    EXPECT_CALL(*connection, send(MsgType(PublishSubscribe::Data{})))
        .Times(0);
    EXPECT_FALSE(t1->publish(Buffer{buffer}));
}

TEST_F(LeafTest, ScatterLeafLogic)
{
    using namespace scatter_gather;

    std::vector<char> data{'a', 'b'};
    Buffer buf{data.data(), data.size()};

    // we are only interested in scatter-gather-related messages
    ignore_messages<ScatterGather::TerminalDescription,
        ScatterGather::TerminalMapping, ScatterGather::TerminalNoticed,
        ScatterGather::TerminalRemoved, ScatterGather::TerminalRemovedAck,
        ScatterGather::BindingDescription, ScatterGather::BindingMapping,
        ScatterGather::BindingNoticed, ScatterGather::BindingRemoved,
        ScatterGather::BindingRemovedAck, ScatterGather::Subscribe,
        ScatterGather::Unsubscribe>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // try to run scatter operation on unbound terminal
    {{
        auto terminal = make_scatter_gather_terminal(Id{1});

        EXPECT_THROW(uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{}),
            api::ExceptionT<CHIRP_ERR_NOT_BOUND>);
    }}

    // run a successful scatter operation
    {{
        auto terminal = make_scatter_gather_terminal(Id{1}, Id{10});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Scatter::create(
            Id{10}, Id{1}, buf))));
        EXPECT_EQ(Id{1}, uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{buf}).first);

        Buffer emptyBuffer;
        EXPECT_CALL(*terminal, on_gathered_message_received_(Id{1},
            GATHER_IGNORED, emptyBuffer))
            .WillOnce(Return(true));
        uut->on_message_received(ScatterGather::Gather::create(
            Id{1}, GATHER_IGNORED, Buffer{}), *connection);

        EXPECT_CALL(*terminal, on_gathered_message_received_(Id{1},
            GATHER_FINISHED, buf))
            .WillOnce(Return(true));
        uut->on_message_received(ScatterGather::Gather::create(
            Id{1}, GATHER_FINISHED, buf), *connection);
    }}

    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run scatter operation and abort gathering messages after the first one
    {{
        auto terminal = make_scatter_gather_terminal(Id{1}, Id{10});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Scatter::create(
            Id{10}, Id{1}, buf))));
        EXPECT_EQ(Id{1}, uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{buf}).first);

        Buffer emptyBuffer;
        EXPECT_CALL(*terminal, on_gathered_message_received_(Id{1},
            GATHER_IGNORED, emptyBuffer))
            .WillOnce(Return(false));
        uut->on_message_received(ScatterGather::Gather::create(
            Id{1}, GATHER_IGNORED, Buffer{}), *connection);

        EXPECT_CALL(*terminal, on_gathered_message_received_(_, _, _))
            .Times(0);
        uut->on_message_received(ScatterGather::Gather::create(
            Id{1}, GATHER_FINISHED, buf), *connection);
    }}

    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run scatter operation and cancel it
    {{
        auto terminal = make_scatter_gather_terminal(Id{1}, Id{11});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Scatter::create(
            Id{11}, Id{1}, buf))));
        EXPECT_EQ(Id{1}, uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{buf}).first);
        uut->scatter_gather::LeafLogic<>::sg_cancel_scatter(*terminal, Id{1});

        uut->on_message_received(ScatterGather::Gather::create(
            Id{1}, GATHER_FINISHED, Buffer{}), *connection);
    }}

    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run scatter operation and destroy the binding
    {{
        auto terminal = make_scatter_gather_terminal(Id{1}, Id{12});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Scatter::create(
            Id{12}, Id{1}, buf))));
        EXPECT_EQ(Id{1}, uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{buf}).first);

        Buffer emptyBuffer;
        EXPECT_CALL(*terminal, on_gathered_message_received_(Id{1},
            GATHER_BINDINGDESTROYED | GATHER_FINISHED, emptyBuffer))
            .WillOnce(Return(true));
        uut->on_message_received(ScatterGather::BindingRemoved::create(
            Id{1}), *connection);
    }}

    // run scatter operation and destroy terminal
    {{
        auto terminal = make_scatter_gather_terminal(Id{1}, Id{13});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Scatter::create(
            Id{13}, Id{1}, buf))));
        EXPECT_EQ(Id{1}, uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{buf}).first);

        Buffer emptyBuffer;
		EXPECT_CALL(*terminal, on_gathered_message_received_(Id{1},
			GATHER_FINISHED, emptyBuffer))
			.WillOnce(Return(false));
        uut->on_message_received(ScatterGather::Gather::create(
            Id{1}, GATHER_FINISHED, Buffer{}), *connection);
    }}

    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run scatter operation and close the connection
    {{
        auto terminal = make_scatter_gather_terminal(Id{1}, Id{13});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Scatter::create(
            Id{13}, Id{1}, buf))));
        EXPECT_EQ(Id{1}, uut->scatter_gather::LeafLogic<>::sg_scatter(
            *terminal, Buffer{buf}).first);

        Buffer emptyBuffer;
        EXPECT_CALL(*terminal, on_gathered_message_received_(Id{1},
            GATHER_CONNECTIONLOST | GATHER_FINISHED, emptyBuffer))
            .WillOnce(Return(true));
        uut->on_connection_destroyed(*connection);
    }}
}

TEST_F(LeafTest, GatherLeafLogic)
{
    using namespace scatter_gather;

    std::vector<char> data{'a', 'b'};
    Buffer buf{data.data(), data.size()};

    // we are only interested in scatter-gather-related messages
    ignore_messages<ScatterGather::TerminalDescription,
        ScatterGather::TerminalMapping, ScatterGather::TerminalNoticed,
        ScatterGather::TerminalRemoved, ScatterGather::TerminalRemovedAck,
        ScatterGather::BindingDescription, ScatterGather::BindingMapping,
        ScatterGather::BindingNoticed, ScatterGather::BindingRemoved,
        ScatterGather::BindingRemovedAck, ScatterGather::Subscribe,
        ScatterGather::Unsubscribe>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // run a successful gather operation
    {{
        auto t1 = make_scatter_gather_terminal(Id{1}, Id{101});
        auto t2 = make_scatter_gather_terminal(Id{2}, Id{102});
        auto b1 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t1, "A");
        auto b2 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t2, "A");
        uut->on_message_received(ScatterGather::BindingMapping::create(
            Id{1}, Id{201}), *connection);

        EXPECT_CALL(*t1, on_scattered_message_received_(Id{88}, buf));
        EXPECT_CALL(*t2, on_scattered_message_received_(Id{88}, buf));
        uut->on_message_received(ScatterGather::Scatter::create(
            Id{1}, Id{88}, buf), *connection);

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Gather::create(
            Id{88}, GATHER_NO_FLAGS, buf))));
        uut->scatter_gather::LeafLogic<>::sg_respond_to_scattered_message(
            *t1, Id{88}, GATHER_NO_FLAGS, true, Buffer{buf});

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Gather::create(
            Id{88}, GATHER_IGNORED | GATHER_FINISHED, Buffer{}))));
        uut->scatter_gather::LeafLogic<>::sg_respond_to_scattered_message(
            *t2, Id{88}, GATHER_IGNORED, true, Buffer{});
    }}

    uut->on_message_received(ScatterGather::BindingRemovedAck::create(
        Id{1}), *connection);
    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{2}), *connection);
    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run gather operation and destroy one of the two bindings
    {{
        auto t1 = make_scatter_gather_terminal(Id{1}, Id{101});
        auto t2 = make_scatter_gather_terminal(Id{2}, Id{102});
        auto b1 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t1, "A");
        auto b2 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t2, "A");
        uut->on_message_received(ScatterGather::BindingMapping::create(
            Id{1}, Id{201}), *connection);

        EXPECT_CALL(*t1, on_scattered_message_received_(Id{88}, buf));
        EXPECT_CALL(*t2, on_scattered_message_received_(Id{88}, buf));
        uut->on_message_received(ScatterGather::Scatter::create(
            Id{1}, Id{88}, buf), *connection);

        b1.reset();

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Gather::create(
            Id{88}, GATHER_FINISHED, buf))));
        uut->scatter_gather::LeafLogic<>::sg_respond_to_scattered_message(
            *t2, Id{88}, GATHER_NO_FLAGS, true, Buffer{buf});

        EXPECT_THROW(uut->scatter_gather::LeafLogic<>::
            sg_respond_to_scattered_message(*t1, Id{88}, GATHER_NO_FLAGS, true,
                Buffer{buf}), api::ExceptionT<CHIRP_ERR_INVALID_ID>);
    }}

    uut->on_message_received(ScatterGather::BindingRemovedAck::create(
        Id{1}), *connection);
    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{2}), *connection);
    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run gather operation and destroy the binding
    {{
        auto t1 = make_scatter_gather_terminal(Id{1}, Id{101});
        auto b1 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t1, "A");
        uut->on_message_received(ScatterGather::BindingMapping::create(
            Id{1}, Id{201}), *connection);

        EXPECT_CALL(*t1, on_scattered_message_received_(Id{88}, buf));
        uut->on_message_received(ScatterGather::Scatter::create(
            Id{1}, Id{88}, buf), *connection);

        EXPECT_CALL(*connection, send(Msg(ScatterGather::Gather::create(
            Id{88}, GATHER_BINDINGDESTROYED | GATHER_FINISHED, Buffer{}))));
        b1.reset();

        EXPECT_THROW(uut->scatter_gather::LeafLogic<>::
            sg_respond_to_scattered_message(*t1, Id{88}, GATHER_NO_FLAGS, true,
                Buffer{buf}), api::ExceptionT<CHIRP_ERR_INVALID_ID>);
    }}

    uut->on_message_received(ScatterGather::BindingRemovedAck::create(
        Id{1}), *connection);
    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run gather operation and unmap the binding
    {{
        auto t1 = make_scatter_gather_terminal(Id{1}, Id{101});
        auto b1 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t1, "A");
        uut->on_message_received(ScatterGather::BindingMapping::create(
            Id{1}, Id{201}), *connection);

        EXPECT_CALL(*t1, on_scattered_message_received_(Id{88}, buf));
        uut->on_message_received(ScatterGather::Scatter::create(
            Id{1}, Id{88}, buf), *connection);

        uut->on_message_received(ScatterGather::TerminalRemoved::create(
            Id{1}), *connection);

        EXPECT_THROW(uut->scatter_gather::LeafLogic<>::
            sg_respond_to_scattered_message(*t1, Id{88}, GATHER_NO_FLAGS, true,
                Buffer{buf}), api::ExceptionT<CHIRP_ERR_INVALID_ID>);
    }}

    uut->on_message_received(ScatterGather::TerminalRemovedAck::create(
        Id{1}), *connection);

    // run gather operation and close the connection
    {{
        auto t1 = make_scatter_gather_terminal(Id{1}, Id{101});
        auto b1 = std::make_shared<BindingMock<scatter_gather::LeafLogic<>>>(
            *t1, "A");
        uut->on_message_received(ScatterGather::BindingMapping::create(
            Id{1}, Id{201}), *connection);

        EXPECT_CALL(*t1, on_scattered_message_received_(Id{88}, buf));
        uut->on_message_received(ScatterGather::Scatter::create(
            Id{1}, Id{88}, buf), *connection);

        uut->on_connection_destroyed(*connection);

        EXPECT_THROW(uut->scatter_gather::LeafLogic<>::
            sg_respond_to_scattered_message(*t1, Id{88}, GATHER_NO_FLAGS, true,
                Buffer{buf}), api::ExceptionT<CHIRP_ERR_INVALID_ID>);
    }}
}

TEST_F(LeafTest, ScatterGatherTerminal)
{
    using namespace scatter_gather;

    std::vector<char> scatData{'a', 'b', 'c'};
    Buffer scatBuf{scatData.data(), scatData.size() - 1};
    Buffer largeScatBuf{scatData.data(), scatData.size()};

    std::vector<char> gathData(2);
    auto gathBuf = boost::asio::buffer(gathData);

    std::vector<char> respData{'X', 'Y'};
    Buffer respBuf{respData.data(), respData.size()};

    auto leaf = std::make_shared<LeafMock>();

    // run scatter-gather operation on unbound terminal
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        EXPECT_CALL(*leaf, scatter_(Ref(*terminal), scatBuf))
            .WillOnce(Throw(api::ExceptionT<CHIRP_ERR_NOT_BOUND>{}));

        bool called = false;
        EXPECT_THROW(terminal->async_scatter_gather(Buffer{scatBuf}, gathBuf,
            [&](const api::Exception& e, Id operationId,
            gather_flags flags, std::size_t size) -> bool {
                called = true;
                return true;
            }
        ), api::ExceptionT<CHIRP_ERR_NOT_BOUND>);

        EXPECT_FALSE(called);
    }}

    // run successful scatter-gather operation
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        EXPECT_CALL(*leaf, scatter_(Ref(*terminal), scatBuf))
            .WillOnce(Return(Id{444}));

        int calls = 0;
        terminal->async_scatter_gather(Buffer{scatBuf}, gathBuf,
            [&](const api::Exception& e, Id operationId,
            gather_flags flags, std::size_t size) -> bool  {
                ++calls;

                EXPECT_EQ(Id{444}, operationId);

                if (calls == 1) {
                    EXPECT_EQ(CHIRP_ERR_BUFFER_TOO_SMALL, e.error_code());
                    EXPECT_EQ(GATHER_NO_FLAGS, flags);
                    EXPECT_EQ(3, size);
                }
                else {
                    EXPECT_EQ(CHIRP_OK, e.error_code());
                    EXPECT_EQ(GATHER_FINISHED, flags);
                    EXPECT_EQ(2, size);
                }

                return true;
            }
        );

        terminal->on_gathered_message_received(Id{444}, GATHER_NO_FLAGS,
            Buffer{largeScatBuf});
        terminal->on_gathered_message_received(Id{444}, GATHER_FINISHED,
            Buffer{scatBuf});
        EXPECT_EQ(2, calls);
    }}

    // run successful scatter-gather operation and abort after the first message
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        EXPECT_CALL(*leaf, scatter_(Ref(*terminal), scatBuf))
            .WillOnce(Return(Id{444}));

        int calls = 0;
        terminal->async_scatter_gather(Buffer{scatBuf}, gathBuf,
            [&](const api::Exception& e, Id operationId,
            gather_flags flags, std::size_t size) -> bool  {
                ++calls;

                EXPECT_EQ(CHIRP_OK, e.error_code());
                EXPECT_EQ(Id{444}, operationId);
                EXPECT_EQ(GATHER_NO_FLAGS, flags);
                EXPECT_EQ(2, size);

                return false;
            }
        );

        terminal->on_gathered_message_received(Id{444}, GATHER_NO_FLAGS,
            Buffer{scatBuf});
        EXPECT_EQ(1, calls);
    }}

    // run scatter-gather operation and cancel it
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        EXPECT_CALL(*leaf, scatter_(Ref(*terminal), scatBuf))
            .WillOnce(Return(Id{444}));

        int calls = 0;
        terminal->async_scatter_gather(Buffer{scatBuf}, gathBuf,
            [&](const api::Exception& e, Id operationId,
            gather_flags flags, std::size_t size) -> bool  {
                ++calls;

                EXPECT_EQ(CHIRP_ERR_CANCELED, e.error_code());
                EXPECT_EQ(Id{444}, operationId);
                EXPECT_EQ(GATHER_NO_FLAGS, flags);
                EXPECT_EQ(0, size);

                return true;
            }
        );

        EXPECT_CALL(*leaf, cancel_scatter_(Ref(*terminal), Id{444}));
        terminal->cancel_scatter_gather(Id{444});
        EXPECT_EQ(1, calls);
    }}

    // run scatter-gather operation and destroy the terminal
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        EXPECT_CALL(*leaf, scatter_(Ref(*terminal), scatBuf))
            .WillOnce(Return(Id{444}));

        int calls = 0;
        terminal->async_scatter_gather(Buffer{scatBuf}, gathBuf,
            [&](const api::Exception& e, Id operationId,
            gather_flags flags, std::size_t size) -> bool  {
                ++calls;

                EXPECT_EQ(CHIRP_ERR_CANCELED, e.error_code());
                EXPECT_EQ(Id{444}, operationId);
                EXPECT_EQ(GATHER_NO_FLAGS, flags);
                EXPECT_EQ(0, size);

                return true;
            }
        );

        terminal.reset();
        EXPECT_EQ(1, calls);
    }}

    // run receive scattered message operation and respond to the message
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        int calls = 0;
        terminal->async_receive_scattered_message(gathBuf,
            [&](const api::Exception& e, Id operationId,
            std::size_t size) {
                ++calls;

                EXPECT_EQ(CHIRP_OK, e.error_code());
                EXPECT_EQ(Id{999}, operationId);
                EXPECT_EQ(2, size);
            }
        );

        terminal->on_scattered_message_received(Id{999}, Buffer{scatBuf});
        EXPECT_EQ(1, calls);

        Buffer buffer{respBuf};
        EXPECT_CALL(*leaf, respond_to_scattered_message_(Ref(*terminal),
            Id{999}, GATHER_NO_FLAGS, true, buffer));
        terminal->respond_to_scattered_message(Id{999}, Buffer{respBuf});
    }}

    // run receive scattered message operation with too much data
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        int calls = 0;
        terminal->async_receive_scattered_message(gathBuf,
            [&](const api::Exception& e, Id operationId,
            std::size_t size) {
                ++calls;

                EXPECT_EQ(CHIRP_ERR_BUFFER_TOO_SMALL, e.error_code());
                EXPECT_EQ(Id{999}, operationId);
                EXPECT_EQ(3, size);
            }
        );

        terminal->on_scattered_message_received(Id{999}, Buffer{largeScatBuf});
        EXPECT_EQ(1, calls);
    }}

    // run receive scattered message operation and ignore the message
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        terminal->async_receive_scattered_message(gathBuf,
            [&](const api::Exception& e, Id operationId,
            std::size_t size) {
            }
        );

        terminal->on_scattered_message_received(Id{999}, Buffer{scatBuf});

        Buffer emptyBuffer;
        EXPECT_CALL(*leaf, respond_to_scattered_message_(Ref(*terminal),
            Id{999}, GATHER_IGNORED, true, emptyBuffer));
        terminal->ignore_scattered_message(Id{999});
    }}

    // check deaf behavior
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        Buffer emptyBuffer;
        EXPECT_CALL(*leaf, respond_to_scattered_message_(Ref(*terminal),
            Id{999}, GATHER_DEAF, false, emptyBuffer));
        terminal->on_scattered_message_received(Id{999}, Buffer{scatBuf});
    }}

    // run receive scattered message operation and cancel it
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        int calls = 0;
        terminal->async_receive_scattered_message(gathBuf,
            [&](const api::Exception& e, Id operationId,
            std::size_t size) {
                ++calls;

                EXPECT_EQ(CHIRP_ERR_CANCELED, e.error_code());
                EXPECT_EQ(Id{}, operationId);
                EXPECT_EQ(0, size);
            }
        );

        terminal->cancel_receive_scattered_message();
        EXPECT_EQ(1, calls);
    }}

    // run receive scattered message operation and destroy the terminal
    {{
        auto terminal = std::make_shared<Terminal<>>(*leaf,
            Identifier{0u, "T", false});

        int calls = 0;
        terminal->async_receive_scattered_message(gathBuf,
            [&](const api::Exception& e, Id operationId,
            std::size_t size) {
                ++calls;

                EXPECT_EQ(CHIRP_ERR_CANCELED, e.error_code());
                EXPECT_EQ(Id{}, operationId);
                EXPECT_EQ(0, size);
            }
        );

        terminal.reset();
        EXPECT_EQ(1, calls);
    }}
}

TEST_F(LeafTest, CachedPublishSubscribePublish)
{
    // we are only interested in cached publish-subscribe-related messages
    ignore_messages<CachedPublishSubscribe::TerminalDescription,
        CachedPublishSubscribe::TerminalMapping,
        CachedPublishSubscribe::TerminalNoticed,
        CachedPublishSubscribe::TerminalRemoved,
        CachedPublishSubscribe::TerminalRemovedAck,
        CachedPublishSubscribe::BindingDescription,
        CachedPublishSubscribe::BindingMapping,
        CachedPublishSubscribe::BindingNoticed,
        CachedPublishSubscribe::BindingRemoved,
        CachedPublishSubscribe::BindingRemovedAck,
        CachedPublishSubscribe::CachedData>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add cached publish-subscribe terminal T1 and map it
    auto t1 = std::make_shared<CachedPublishSubscribeTerminalMock>(*uut,
        Identifier{0u, "T1", false});
    uut->on_message_received(CachedPublishSubscribe::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // send a message via T1
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};

    EXPECT_CALL(*connection, send(Msg(CachedPublishSubscribe::Data::create(
        Id{123}, buffer))));
    EXPECT_TRUE(t1->publish(Buffer{buffer}));

    // de-register the leaf connection
    uut->on_connection_destroyed(*connection);

    // register a node connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(true));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);
    uut->on_message_received(CachedPublishSubscribe::TerminalMapping::create(
        Id{1}, Id{456}), *connection);

    // send a message via T1
    EXPECT_CALL(*connection, send(MsgType(CachedPublishSubscribe::Data{})))
        .Times(0);
    EXPECT_FALSE(t1->publish(Buffer{buffer}));

    // subscribe and send a message via T1
    uut->on_message_received(CachedPublishSubscribe::Subscribe::create(
        Id{1}), *connection);

    EXPECT_CALL(*connection, send(Msg(CachedPublishSubscribe::Data::create(
        Id{456}, buffer))));
    EXPECT_TRUE(t1->publish(Buffer{buffer}));

    // unsubscribe and send a message via T1
    uut->on_message_received(CachedPublishSubscribe::Unsubscribe::create(
        Id{1}), *connection);

    EXPECT_CALL(*connection, send(MsgType(CachedPublishSubscribe::Data{})))
        .Times(0);
    EXPECT_FALSE(t1->publish(Buffer{buffer}));
}

TEST_F(LeafTest, CachedPublishSubscribeCacheLeafConnection)
{
    // we are only interested in CachedData message
    ignore_messages<CachedPublishSubscribe::TerminalDescription,
        CachedPublishSubscribe::TerminalMapping,
        CachedPublishSubscribe::TerminalNoticed,
        CachedPublishSubscribe::TerminalRemoved,
        CachedPublishSubscribe::TerminalRemovedAck,
        CachedPublishSubscribe::BindingDescription,
        CachedPublishSubscribe::BindingMapping,
        CachedPublishSubscribe::BindingNoticed,
        CachedPublishSubscribe::BindingRemoved,
        CachedPublishSubscribe::BindingRemovedAck,
        CachedPublishSubscribe::Data>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add cached publish-subscribe terminal T1 and map it
    EXPECT_CALL(*connection, send(MsgType(CachedPublishSubscribe::CachedData{})))
        .Times(0);
    auto t1 = std::make_shared<CachedPublishSubscribeTerminalMock>(*uut,
        Identifier{0u, "T1", false});
    uut->on_message_received(CachedPublishSubscribe::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // send a message via T1
    std::vector<char> data1{'a', 'b'};
    Buffer buffer1{data1.data(), data1.size()};
    EXPECT_TRUE(t1->publish(Buffer{buffer1}));

    // check that the cache is empty
    char cache[2];
    auto cacheRes = t1->get_cache(boost::asio::buffer(cache, sizeof(cache)));
    EXPECT_FALSE(cacheRes.first);
    EXPECT_EQ(0, cacheRes.second);

    // create a binding so we can receive messages on T1
    auto b1 = std::make_shared<BindingMock<
        cached_publish_subscribe::LeafLogic<>>>(*t1, "ABC");

    // receive a data message
    std::vector<char> data2{'c', 'd'};
    Buffer buffer2{data2.data(), data2.size()};
    uut->on_message_received(CachedPublishSubscribe::Data::create(
        Id{1}, buffer2), *connection);

    // check that cache is not empty any more
    cacheRes = t1->get_cache(boost::asio::buffer(cache, sizeof(cache)));
    EXPECT_TRUE(cacheRes.first);
    EXPECT_EQ(2, cacheRes.second);
    EXPECT_EQ(data2[0], cache[0]);
    EXPECT_EQ(data2[1], cache[1]);

    // receive a cached data message
    uut->on_message_received(CachedPublishSubscribe::CachedData::create(
        Id{1}, buffer1), *connection);

    // check the cache
    cacheRes = t1->get_cache(boost::asio::buffer(cache, sizeof(cache)));
    EXPECT_TRUE(cacheRes.first);
    EXPECT_EQ(2, cacheRes.second);
    EXPECT_EQ(data1[0], cache[0]);
    EXPECT_EQ(data1[1], cache[1]);

    EXPECT_THROW(t1->get_cache(boost::asio::buffer(cache, 1)),
        api::ExceptionT<CHIRP_ERR_BUFFER_TOO_SMALL>);

    // un-map the terminal
    uut->on_message_received(CachedPublishSubscribe::BindingRemoved::create(
        Id{1}), *connection);

    // map the terminal
    EXPECT_CALL(*connection, send(MsgType(CachedPublishSubscribe::Data{})))
        .Times(0);
    EXPECT_CALL(*connection, send(Msg(CachedPublishSubscribe::CachedData::create(
        Id{456}, buffer1))));
    uut->on_message_received(CachedPublishSubscribe::BindingDescription::create(
        Identifier{0u, "T1", false}, Id{456}), *connection);
}

TEST_F(LeafTest, CachedPublishSubscribeCacheNodeConnection)
{
    // we are only interested in CachedData message
    ignore_messages<CachedPublishSubscribe::TerminalDescription,
        CachedPublishSubscribe::TerminalMapping,
        CachedPublishSubscribe::TerminalNoticed,
        CachedPublishSubscribe::TerminalRemoved,
        CachedPublishSubscribe::TerminalRemovedAck,
        CachedPublishSubscribe::BindingDescription,
        CachedPublishSubscribe::BindingMapping,
        CachedPublishSubscribe::BindingNoticed,
        CachedPublishSubscribe::BindingRemoved,
        CachedPublishSubscribe::BindingRemovedAck,
        CachedPublishSubscribe::Data>();

    // register a node connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(true));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add cached publish-subscribe terminal T1 and map it
    auto t1 = std::make_shared<CachedPublishSubscribeTerminalMock>(*uut,
        Identifier{0u, "T1", false});
    uut->on_message_received(CachedPublishSubscribe::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // subscribe to T1
    EXPECT_CALL(*connection, send(MsgType(CachedPublishSubscribe::CachedData{})))
        .Times(0);
    uut->on_message_received(CachedPublishSubscribe::Subscribe::create(
        Id{1}), *connection);

    // send a message via T1
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};
    EXPECT_TRUE(t1->publish(Buffer{buffer}));

    // un-subscribe from T1
    uut->on_message_received(CachedPublishSubscribe::Unsubscribe::create(
        Id{1}), *connection);

    // re-subscribe to T1
    EXPECT_CALL(*connection, send(Msg(CachedPublishSubscribe::CachedData::create(
        Id{123}, buffer))));
    uut->on_message_received(CachedPublishSubscribe::Subscribe::create(
        Id{1}), *connection);

    // de-register the node connection
    uut->on_connection_destroyed(*connection);

    // re-register a node connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*connection, send(MsgType(CachedPublishSubscribe::CachedData{})))
        .Times(0);
    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);
    uut->on_message_received(CachedPublishSubscribe::TerminalMapping::create(
        Id{1}, Id{456}), *connection);
}

TEST_F(LeafTest, MasterTerminal)
{
    // we are only interested in data messages
    ignore_messages<MasterSlave::TerminalDescription,
        MasterSlave::TerminalMapping, MasterSlave::TerminalNoticed,
        MasterSlave::TerminalRemoved, MasterSlave::TerminalRemovedAck,
        MasterSlave::BindingDescription, MasterSlave::BindingMapping,
        MasterSlave::BindingNoticed, MasterSlave::BindingRemoved,
        MasterSlave::BindingRemovedAck, MasterSlave::TerminalMapping>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add master terminal T1 and map it
    auto t1 = std::make_shared<MasterSlaveTerminalMock>(*uut,
        Identifier{0u, "T1", false});
    uut->on_message_received(MasterSlave::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // create a binding so we can receive messages on T1
    auto b1 = std::make_shared<BindingMock<
        master_slave::LeafLogic<>>>(*t1, "ABC");

    // receive a data message and check that it re-publishes the message
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};

    EXPECT_CALL(*connection, send(Msg(MasterSlave::Data::create(
        Id{123}, buffer))));
    uut->on_message_received(MasterSlave::Data::create(
        Id{1}, buffer), *connection);

    // unmap T1
    uut->on_message_received(MasterSlave::BindingRemoved::create(
        Id{1}), *connection);

    // receive a data message and check that it does not re-publish the message
    EXPECT_CALL(*connection, send(MsgType(MasterSlave::Data{})))
        .Times(0);
    uut->on_message_received(MasterSlave::Data::create(
        Id{1}, buffer), *connection);
}

TEST_F(LeafTest, SlaveTerminal)
{
    // we are only interested in data messages
    ignore_messages<MasterSlave::TerminalDescription,
        MasterSlave::TerminalMapping, MasterSlave::TerminalNoticed,
        MasterSlave::TerminalRemoved, MasterSlave::TerminalRemovedAck,
        MasterSlave::BindingDescription, MasterSlave::BindingMapping,
        MasterSlave::BindingNoticed, MasterSlave::BindingRemoved,
        MasterSlave::BindingRemovedAck, MasterSlave::TerminalMapping>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add slave terminal T1 and map it
    auto t1 = std::make_shared<MasterSlaveTerminalMock>(*uut,
        Identifier{0u, "T1", true});
    uut->on_message_received(MasterSlave::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // create a binding so we can receive messages on T1
    auto b1 = std::make_shared<BindingMock<
        master_slave::LeafLogic<>>>(*t1, "ABC");

    // receive a data message and check that it does not re-publish the message
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};

    EXPECT_CALL(*connection, send(MsgType(MasterSlave::Data{})))
        .Times(0);
    uut->on_message_received(MasterSlave::Data::create(
        Id{1}, buffer), *connection);
}

TEST_F(LeafTest, CachedMasterTerminal)
{
    // we are only interested in data messages
    ignore_messages<CachedMasterSlave::TerminalDescription,
        CachedMasterSlave::TerminalMapping, CachedMasterSlave::TerminalNoticed,
        CachedMasterSlave::TerminalRemoved, CachedMasterSlave::TerminalRemovedAck,
        CachedMasterSlave::BindingDescription, CachedMasterSlave::BindingMapping,
        CachedMasterSlave::BindingNoticed, CachedMasterSlave::BindingRemoved,
        CachedMasterSlave::BindingRemovedAck, CachedMasterSlave::TerminalMapping>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add master terminal T1 and map it
    auto t1 = std::make_shared<CachedMasterSlaveTerminalMock>(*uut,
        Identifier{0u, "T1", false});
    uut->on_message_received(CachedMasterSlave::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // create a binding so we can receive messages on T1
    auto b1 = std::make_shared<BindingMock<
        cached_master_slave::LeafLogic<>>>(*t1, "ABC");

    // receive a data message and check that it re-publishes the message
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};

    EXPECT_CALL(*connection, send(Msg(CachedMasterSlave::Data::create(
        Id{123}, buffer))));
    uut->on_message_received(CachedMasterSlave::Data::create(
        Id{1}, buffer), *connection);

    // receive a cached data message and check that it re-publishes the message
    EXPECT_CALL(*connection, send(Msg(CachedMasterSlave::CachedData::create(
        Id{123}, buffer))));
    uut->on_message_received(CachedMasterSlave::CachedData::create(
        Id{1}, buffer), *connection);

    // unmap T1
    uut->on_message_received(CachedMasterSlave::BindingRemoved::create(
        Id{1}), *connection);

    // receive a data message and check that it does not re-publish the message
    EXPECT_CALL(*connection, send(MsgType(CachedMasterSlave::Data{})))
        .Times(0);
    uut->on_message_received(CachedMasterSlave::Data::create(
        Id{1}, buffer), *connection);

    // receive a cached data message and check that it does not re-publish the
    // message
    EXPECT_CALL(*connection, send(MsgType(CachedMasterSlave::CachedData{})))
        .Times(0);
    uut->on_message_received(CachedMasterSlave::CachedData::create(
        Id{1}, buffer), *connection);
}

TEST_F(LeafTest, CachedSlaveTerminal)
{
    // we are only interested in data messages
    ignore_messages<CachedMasterSlave::TerminalDescription,
        CachedMasterSlave::TerminalMapping, CachedMasterSlave::TerminalNoticed,
        CachedMasterSlave::TerminalRemoved, CachedMasterSlave::TerminalRemovedAck,
        CachedMasterSlave::BindingDescription, CachedMasterSlave::BindingMapping,
        CachedMasterSlave::BindingNoticed, CachedMasterSlave::BindingRemoved,
        CachedMasterSlave::BindingRemovedAck, CachedMasterSlave::TerminalMapping>();

    // register a leaf connection
    EXPECT_CALL(*connection, remote_is_node())
        .WillRepeatedly(Return(false));

    uut->on_new_connection(*connection);
    uut->on_connection_started(*connection);

    // add slave terminal T1 and map it
    auto t1 = std::make_shared<CachedMasterSlaveTerminalMock>(*uut,
        Identifier{0u, "T1", true});
    uut->on_message_received(CachedMasterSlave::TerminalMapping::create(
        Id{1}, Id{123}), *connection);

    // create a binding so we can receive messages on T1
    auto b1 = std::make_shared<BindingMock<
        cached_master_slave::LeafLogic<>>>(*t1, "ABC");

    // receive a data message and check that it does not re-publish the message
    std::vector<char> data{'a', 'b'};
    Buffer buffer{data.data(), data.size()};

    EXPECT_CALL(*connection, send(MsgType(CachedMasterSlave::Data{})))
        .Times(0);
    uut->on_message_received(CachedMasterSlave::Data::create(
        Id{1}, buffer), *connection);

    // receive a cached data message and check that it does not re-publish the
    // message
    EXPECT_CALL(*connection, send(MsgType(CachedMasterSlave::CachedData{})))
        .Times(0);
    uut->on_message_received(CachedMasterSlave::CachedData::create(
        Id{1}, buffer), *connection);
}
