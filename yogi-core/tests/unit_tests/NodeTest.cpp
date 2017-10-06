#include "../../src/core/Node.hpp"
using namespace yogi::base;
using namespace yogi::core;
using namespace yogi::messaging::messages;

#include "../mocks/SchedulerMock.hpp"
#include "../mocks/ConnectionMock.hpp"
#include "../mocks/MessageMock.hpp"
using namespace mocks;


struct NodeTest : public testing::Test
{
    typedef std::shared_ptr<testing::StrictMock<ConnectionMock>>
        connection_mock_ptr;

    std::shared_ptr<SchedulerMock> scheduler;
    std::shared_ptr<Node>          uut;
    connection_mock_ptr            node1;
    connection_mock_ptr            node2;
    connection_mock_ptr            leafA;
    connection_mock_ptr            leafB;

    virtual void SetUp() override
    {
        // create a scheduler
        scheduler = std::make_shared<SchedulerMock>();

        // create the UUT and leafs/nodes that we connect to the UUT
        uut    = std::make_shared<Node>(*scheduler);
        node1  = std::make_shared<testing::StrictMock<ConnectionMock>>(true);
        node2  = std::make_shared<testing::StrictMock<ConnectionMock>>(true);
        leafA  = std::make_shared<testing::StrictMock<ConnectionMock>>(false);
        leafB  = std::make_shared<testing::StrictMock<ConnectionMock>>(false);

        // connect the leafs/nodes to the node under test
        //
        //         node1
        //           |
        // leafA -- uut -- leafB
        //           |
        //         node2
        uut->on_new_connection(*node1);
        uut->on_connection_started(*node1);
        uut->on_new_connection(*node2);
        uut->on_connection_started(*node2);
        uut->on_new_connection(*leafA);
        uut->on_connection_started(*leafA);
        uut->on_new_connection(*leafB);
        uut->on_connection_started(*leafB);
    }

    static Identifier ident(const char* name)
    {
        return Identifier{0u, name, false};
    }

    template <typename TFirstMessage, typename... TRemainingMessages>
    struct _ignore_messages
    {
        void operator() (NodeTest& test) const
        {
            _ignore_messages<TFirstMessage>()(test);
            _ignore_messages<TRemainingMessages...>()(test);
        }
    };

    template <typename TFirstMessage>
    struct _ignore_messages<TFirstMessage>
    {
        void operator() (NodeTest& test) const
        {
            EXPECT_CALL(*test.leafA, send(MsgType(TFirstMessage{})))
                .Times(AnyNumber());
            EXPECT_CALL(*test.leafB, send(MsgType(TFirstMessage{})))
                .Times(AnyNumber());
            EXPECT_CALL(*test.node1, send(MsgType(TFirstMessage{})))
                .Times(AnyNumber());
            EXPECT_CALL(*test.node2, send(MsgType(TFirstMessage{})))
                .Times(AnyNumber());
        }
    };

    template <typename... TMessages>
    void ignore_messages()
    {
        _ignore_messages<TMessages...>()(*this);
    }

	Buffer prepare_scatter_gather_test()
	{
		// we are only interested in scatter-gather related messages
		ignore_messages<ScatterGather::TerminalDescription,
			ScatterGather::TerminalMapping, ScatterGather::TerminalNoticed,
			ScatterGather::TerminalRemoved, ScatterGather::TerminalRemovedAck,
			ScatterGather::BindingDescription, ScatterGather::BindingMapping,
			ScatterGather::BindingNoticed, ScatterGather::BindingRemoved,
			ScatterGather::BindingRemovedAck, ScatterGather::Subscribe,
			ScatterGather::Unsubscribe, ScatterGather::BindingEstablished,
			ScatterGather::BindingReleased>();

		// create terminal "a" on node1, leafA and leafB
		uut->on_message_received(ScatterGather::TerminalDescription::create(
			ident("a"), Id{104}), *node1);
		uut->on_message_received(ScatterGather::TerminalMapping::create(
			Id{1}, Id{204}), *node2);
		uut->on_message_received(ScatterGather::TerminalDescription::create(
			ident("a"), Id{13}), *leafA);
		uut->on_message_received(ScatterGather::TerminalMapping::create(
			Id{1}, Id{104}), *node1);
		uut->on_message_received(ScatterGather::TerminalDescription::create(
			ident("a"), Id{23}), *leafB);

		// subscribe to "a" from node1
		uut->on_message_received(ScatterGather::Subscribe::create(Id{1}), *node1);

		// create binding "a" on leafA and leafB
		uut->on_message_received(ScatterGather::BindingDescription::create(
			ident("a"), Id{51}), *leafA);
		uut->on_message_received(ScatterGather::BindingDescription::create(
			ident("a"), Id{61}), *leafB);

		// create buffer with test data
		std::vector<char> data{'a', 'b'};
		return Buffer{data.data(), data.size()};
	}

    void check_publish_subscribe_data_distribution(connection_mock_ptr origin,
        std::initializer_list<connection_mock_ptr> receivers)
    {
        std::map<decltype(node1), Id> idMap;
        idMap[node1] = Id{101};
        idMap[leafA] = Id{17};
        idMap[leafB] = Id{15};

        std::vector<char> data{'a', 'b'};
        Buffer buffer{data.data(), data.size()};

        for (auto x : receivers) {
            auto idIt = idMap.find(x);
            EXPECT_CALL(*x, send(Msg(PublishSubscribe::Data::create(
                idIt->second, buffer))));
            idMap.erase(idIt);
        }

        for (auto x : idMap) {
            EXPECT_CALL(*x.first, send(MsgType(PublishSubscribe::Data{})))
                .Times(0);
        }

        uut->on_message_received(PublishSubscribe::Data::create(
            Id{1}, buffer), *origin);
    }
};

TEST_F(NodeTest, NewTerminalOnLeaf)
{
    // create the terminal "a" on leafA
    EXPECT_CALL(*leafA, send(Msg(DeafMute::TerminalMapping::create(
        Id{3}, Id{1}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{3}), *leafA);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{18}), *node1);
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{93}), *node2);

    // create the same terminal on leafB
    EXPECT_CALL(*leafB, send(Msg(DeafMute::TerminalMapping::create(
        Id{7}, Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{7}), *leafB);

    // remove the terminal from leafA
    EXPECT_CALL(*leafA, send(Msg(DeafMute::TerminalRemovedAck::create(Id{3}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *leafA);

    // remove the terminal from leafB
    EXPECT_CALL(*leafB, send(Msg(DeafMute::TerminalRemovedAck::create(Id{7}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemoved::create(Id{18}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemoved::create(Id{93}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *leafB);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node1);
    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node2);
}

TEST_F(NodeTest, NewTerminalOnNode)
{
    // create an terminal on node1
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalMapping::create(
        Id{4}, Id{1}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{4}), *node1);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{55}), *node2);

    // create the same terminal on node2
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalMapping::create(
        Id{55}, Id{1}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{55}), *node2);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{4}), *node1);

    // remove the terminal from node1
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemovedAck::create(Id{4}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemoved::create(Id{55}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node1);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node2);

    // remove the terminal from node2
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemovedAck::create(
        Id{55}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemoved::create(Id{4}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node2);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node1);
}

TEST_F(NodeTest, Bindings)
{
    // we are only interested in binding messages
    ignore_messages<DeafMute::TerminalDescription, DeafMute::TerminalMapping,
        DeafMute::TerminalNoticed, DeafMute::TerminalRemoved,
        DeafMute::TerminalRemovedAck>();

    // remove node2
    uut->on_connection_destroyed(*node2);

    // create the terminals "a" on leafA, "b" on LeafB and "b" on node1
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{12}), *leafA);
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{101}), *node1);

    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("b"), Id{18}), *leafB);
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{2}, Id{102}), *node1);

    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("b"), Id{102}), *node1);

    // create a binding to "a" on leafA
    EXPECT_CALL(*leafA, send(Msg(DeafMute::BindingMapping::create(
        Id{5}, Id{1}))));
    uut->on_message_received(DeafMute::BindingDescription::create(
        ident("a"), Id{5}), *leafA);

    // create a binding to "a" on leafB
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingMapping::create(
        Id{3}, Id{1}))));
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingEstablished::create(Id{3}))));
    uut->on_message_received(DeafMute::BindingDescription::create(
        ident("a"), Id{3}), *leafB);

    // create a binding to "b" on leafB
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingMapping::create(
        Id{8}, Id{2}))));
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingEstablished::create(Id{8}))));
    uut->on_message_received(DeafMute::BindingDescription::create(
        ident("b"), Id{8}), *leafB);

    // remove terminal "b" from node1
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingReleased::create(Id{8}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{2}), *node1);

    // create terminal "a" on node1
    EXPECT_CALL(*leafA, send(Msg(DeafMute::BindingEstablished::create(Id{5}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{101}), *node1);

    // remove terminal "a" from leafA
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *leafA);

    // create terminal "b" on leafA
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingEstablished::create(Id{8}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("b"), Id{33}), *leafA);

    // remove terminal "a" from node1
    EXPECT_CALL(*leafA, send(Msg(DeafMute::BindingReleased::create(Id{5}))));
    EXPECT_CALL(*leafB, send(Msg(DeafMute::BindingReleased::create(Id{3}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node1);
}

TEST_F(NodeTest, Connections)
{
    // create terminal "a" on leafA
    EXPECT_CALL(*leafA, send(Msg(DeafMute::TerminalMapping::create(
        Id{3}, Id{1}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{3}), *leafA);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{18}), *node1);
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{93}), *node2);

    // create terminal "b" on node1
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalMapping::create(
        Id{7}, Id{2}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("b"), Id{2}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("b"), Id{7}), *node1);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{2}, Id{55}), *node2);

    // create and connect a new node
    auto node = std::make_shared<StrictMock<ConnectionMock>>(true);
    EXPECT_CALL(*node, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    EXPECT_CALL(*node, send(Msg(DeafMute::TerminalDescription::create(
        ident("b"), Id{2}))));
    uut->on_new_connection(*node);
    uut->on_connection_started(*node);

    // create terminal "c" on the new node
    EXPECT_CALL(*node, send(Msg(DeafMute::TerminalMapping::create(
        Id{8}, Id{3}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("c"), Id{3}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("c"), Id{3}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("c"), Id{8}), *node);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{3}, Id{65}), *node1);
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{3}, Id{74}), *node2);

    // disconnect the new node
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemoved::create(Id{65}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemoved::create(Id{74}))));
    uut->on_connection_destroyed(*node);

    // create and connect a new leaf
    auto leaf = std::make_shared<StrictMock<ConnectionMock>>(false);
    uut->on_new_connection(*leaf);
    uut->on_connection_started(*leaf);

    // create the terminal "b" on the new leaf
    EXPECT_CALL(*leaf, send(Msg(DeafMute::TerminalMapping::create(
        Id{4}, Id{2}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("b"), Id{2}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("b"), Id{4}), *leaf);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{2}, Id{7}), *node1);

    // disconnect the new leaf
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemoved::create(Id{7}))));
    uut->on_connection_destroyed(*leaf);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{2}), *node1);
}

TEST_F(NodeTest, TerminalRemovedBeforeMappingReceived)
{
    // create the terminal "a" on node1 and delay reception of the mapping
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalMapping::create(
        Id{18}, Id{1}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{18}), *node1);

    // remove the terminal from node1
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemovedAck::create(
        Id{18}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node1);

    // now receive the mapping
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemoved::create(Id{99}))));
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{99}), *node2);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node2);
}

TEST_F(NodeTest, TakeOwnership)
{
    // create the terminal "a" on node1
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalMapping::create(
        Id{5}, Id{1}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{5}), *node1);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{105}), *node2);

    // create the same terminal on node2
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalMapping::create(
        Id{105}, Id{1}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{105}), *node2);

    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{5}), *node1);

    // remove the terminal from node2
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemovedAck::create(
        Id{105}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemoved::create(Id{5}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node2);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node1);

    // create the terminal on node2 again but delay the TerminalMapping message
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalMapping::create(
        Id{107}, Id{1}))));
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalDescription::create(
        ident("a"), Id{1}))));
    uut->on_message_received(DeafMute::TerminalDescription::create(
        ident("a"), Id{107}), *node2);

    // remove the terminal from node2 before the TerminalMapping is received
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemovedAck::create(
        Id{107}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node2);

    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemoved::create(Id{5}))));
    uut->on_message_received(DeafMute::TerminalMapping::create(
        Id{1}, Id{5}), *node1);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node1);

    // remove the terminal from node1
    EXPECT_CALL(*node1, send(Msg(DeafMute::TerminalRemovedAck::create(Id{5}))));
    EXPECT_CALL(*node2, send(Msg(DeafMute::TerminalRemoved::create(Id{105}))));
    uut->on_message_received(DeafMute::TerminalRemoved::create(Id{1}), *node1);

    uut->on_message_received(DeafMute::TerminalRemovedAck::create(
        Id{1}), *node2);
}

TEST_F(NodeTest, PublishSubscribe)
{
    // we are only interested in publish-subscribe-related messages
    ignore_messages<PublishSubscribe::TerminalDescription,
        PublishSubscribe::TerminalMapping, PublishSubscribe::TerminalNoticed,
        PublishSubscribe::TerminalRemoved, PublishSubscribe::TerminalRemovedAck,
        PublishSubscribe::BindingDescription, PublishSubscribe::BindingMapping,
        PublishSubscribe::BindingNoticed, PublishSubscribe::BindingRemoved,
        PublishSubscribe::BindingRemovedAck>();

    // create terminal [4] and binding [17] on leafA
    uut->on_message_received(PublishSubscribe::TerminalDescription::create(
        ident("a"), Id{4}), *leafA);
    uut->on_message_received(PublishSubscribe::TerminalMapping::create(
        Id{1}, Id{101}), *node1);
    uut->on_message_received(PublishSubscribe::TerminalMapping::create(
        Id{1}, Id{104}), *node2);

    uut->on_message_received(PublishSubscribe::BindingDescription::create(
        ident("a"), Id{17}), *leafA);

    // make node1 owner of the terminal [101]
    EXPECT_CALL(*leafA, send(Msg(PublishSubscribe::BindingEstablished::create(
        Id{17}))));
    EXPECT_CALL(*node1, send(Msg(PublishSubscribe::Subscribe::create(
        Id{101}))));
    uut->on_message_received(PublishSubscribe::TerminalDescription::create(
        ident("a"), Id{101}), *node1);

    check_publish_subscribe_data_distribution(node1, {leafA});

    // create subscription on node1
    EXPECT_CALL(*leafA, send(Msg(PublishSubscribe::Subscribe::create(Id{4}))));
    uut->on_message_received(PublishSubscribe::Subscribe::create(
        Id{1}), *node1);

    check_publish_subscribe_data_distribution(leafA, {node1});
    check_publish_subscribe_data_distribution(node1, {leafA});

    // create terminal [3] and binding [15] on leafB
    EXPECT_CALL(*leafB, send(Msg(PublishSubscribe::Subscribe::create(Id{3}))));
    uut->on_message_received(PublishSubscribe::TerminalDescription::create(
        ident("a"), Id{3}), *leafB);

    EXPECT_CALL(*leafB, send(Msg(PublishSubscribe::BindingEstablished::create(
        Id{15}))));
    uut->on_message_received(PublishSubscribe::BindingDescription::create(
        ident("a"), Id{15}), *leafB);

    check_publish_subscribe_data_distribution(leafA, {leafB, node1});
    check_publish_subscribe_data_distribution(leafB, {leafA, node1});
    check_publish_subscribe_data_distribution(node1, {leafA, leafB});

    // remove binding from leafB
    uut->on_message_received(PublishSubscribe::BindingRemoved::create(
        Id{1}), *leafB);

    // remove terminal from leafB
    uut->on_message_received(PublishSubscribe::TerminalRemoved::create(
        Id{1}), *leafB);

    check_publish_subscribe_data_distribution(leafA, {node1});
    check_publish_subscribe_data_distribution(node1, {leafA});

    // remove subscription from node1
    EXPECT_CALL(*leafA, send(Msg(PublishSubscribe::Unsubscribe::create(
        Id{4}))));
    uut->on_message_received(PublishSubscribe::Unsubscribe::create(
        Id{1}), *node1);

    check_publish_subscribe_data_distribution(node1, {leafA});

    // re-create subscription on node1
    EXPECT_CALL(*leafA, send(Msg(PublishSubscribe::Subscribe::create(Id{4}))));
    uut->on_message_received(PublishSubscribe::Subscribe::create(
        Id{1}), *node1);

    // close node1 connetion
    EXPECT_CALL(*leafA, send(Msg(PublishSubscribe::BindingReleased::create(
        Id{17}))));
    EXPECT_CALL(*leafA, send(Msg(PublishSubscribe::Unsubscribe::create(
        Id{4}))));
    uut->on_connection_destroyed(*node1);
}

TEST_F(NodeTest, CachedPublishSubscribe)
{
    // we are only interested in the CachedData message
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
        CachedPublishSubscribe::BindingEstablished,
        CachedPublishSubscribe::BindingReleased,
        CachedPublishSubscribe::Subscribe,
        CachedPublishSubscribe::Unsubscribe,
        CachedPublishSubscribe::Data>();

    // create terminal [4] on leafA
    uut->on_message_received(CachedPublishSubscribe::TerminalDescription::create(
        ident("a"), Id{4}), *leafA);

    uut->on_message_received(CachedPublishSubscribe::TerminalMapping::create(
        Id{1}, Id{101}), *node1);

    // create binding [17] on leafA
    uut->on_message_received(CachedPublishSubscribe::BindingDescription::create(
        ident("a"), Id{17}), *leafA);

    // make node1 owner of the terminal [101]
    uut->on_message_received(CachedPublishSubscribe::TerminalDescription::create(
        ident("a"), Id{104}), *node1);

    // subscribe from node1
    EXPECT_CALL(*node1, send(MsgType(CachedPublishSubscribe::CachedData{})))
        .Times(0);
    uut->on_message_received(CachedPublishSubscribe::Subscribe::create(
        Id{1}), *node1);

    // send a message via terminal [4] on leafA
    std::vector<char> data1{'a', 'b'};
    Buffer buffer1{data1.data(), data1.size()};
    uut->on_message_received(CachedPublishSubscribe::Data::create(
        Id{1}, buffer1), *leafA);

    // un-subscribe and re-subscribe from node1
    uut->on_message_received(CachedPublishSubscribe::Unsubscribe::create(
        Id{1}), *node1);
    EXPECT_CALL(*node1, send(Msg(CachedPublishSubscribe::CachedData::create(
        Id{101}, buffer1))));
    uut->on_message_received(CachedPublishSubscribe::Subscribe::create(
        Id{1}), *node1);

    // send cached data from node1
    std::vector<char> data2{'c', 'd'};
    Buffer buffer2{data2.data(), data2.size()};
    EXPECT_CALL(*leafA, send(Msg(CachedPublishSubscribe::CachedData::create(
        Id{17}, buffer2))));
    uut->on_message_received(CachedPublishSubscribe::CachedData::create(
        Id{1}, buffer2), *node1);

    // un-subscribe and re-subscribe from node1
    uut->on_message_received(CachedPublishSubscribe::Unsubscribe::create(
        Id{1}), *node1);
    EXPECT_CALL(*node1, send(Msg(CachedPublishSubscribe::CachedData::create(
        Id{101}, buffer2))));
    uut->on_message_received(CachedPublishSubscribe::Subscribe::create(
        Id{1}), *node1);
}

TEST_F(NodeTest, SuccessfulScatterGatherOverNode)
{
    using namespace scatter_gather;
	Buffer buffer = prepare_scatter_gather_test();

    // run successful scatter-gather operation over node1
    EXPECT_CALL(*leafA, send(Msg(ScatterGather::Scatter::create(
        Id{51}, Id{1}, buffer))));
    EXPECT_CALL(*leafB, send(Msg(ScatterGather::Scatter::create(
        Id{61}, Id{1}, buffer))));
    uut->on_message_received(ScatterGather::Scatter::create(
        Id{1}, Id{555}, buffer), *node1);

    EXPECT_CALL(*node1, send(Msg(ScatterGather::Gather::create(
        Id{555}, GATHER_IGNORED, buffer))));
    uut->on_message_received(ScatterGather::Gather::create(
        Id{1}, GATHER_IGNORED, buffer), *leafA);

    EXPECT_CALL(*node1, send(Msg(ScatterGather::Gather::create(
        Id{555}, GATHER_NO_FLAGS, buffer))));
    uut->on_message_received(ScatterGather::Gather::create(
        Id{1}, GATHER_FINISHED, buffer), *leafA);

    EXPECT_CALL(*node1, send(Msg(ScatterGather::Gather::create(
        Id{555}, GATHER_FINISHED, buffer))));
    uut->on_message_received(ScatterGather::Gather::create(
        Id{1}, GATHER_FINISHED, buffer), *leafB);
}

TEST_F(NodeTest, SuccessfulScatterGatherOverLeaf)
{
	using namespace scatter_gather;
	Buffer buffer = prepare_scatter_gather_test();

	// run successful scatter-gather operation over leafA
	EXPECT_CALL(*node1, send(Msg(ScatterGather::Scatter::create(
		Id{104}, Id{1}, buffer))));
	EXPECT_CALL(*leafB, send(Msg(ScatterGather::Scatter::create(
		Id{61}, Id{1}, buffer))));
	uut->on_message_received(ScatterGather::Scatter::create(
		Id{1}, Id{555}, buffer), *leafA);

	EXPECT_CALL(*leafA, send(Msg(ScatterGather::Gather::create(
		Id{555}, GATHER_NO_FLAGS, buffer))));
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_FINISHED, buffer), *node1);

	EXPECT_CALL(*leafA, send(Msg(ScatterGather::Gather::create(
		Id{555}, GATHER_FINISHED, buffer))));
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_FINISHED, buffer), *leafB);
}

TEST_F(NodeTest, RemoveBindingsAndSubscriptionsWhileScatterGatherOverLeaf)
{
	using namespace scatter_gather;
	Buffer buffer = prepare_scatter_gather_test();

	// run scatter-gather operation over leafA and remove bindings/subscriptions
	EXPECT_CALL(*node1, send(Msg(ScatterGather::Scatter::create(
		Id{104}, Id{1}, buffer))));
	EXPECT_CALL(*leafB, send(Msg(ScatterGather::Scatter::create(
		Id{61}, Id{1}, buffer))));
	uut->on_message_received(ScatterGather::Scatter::create(
		Id{1}, Id{555}, buffer), *leafA);

	EXPECT_CALL(*leafA, send(Msg(ScatterGather::Gather::create(
		Id{555}, GATHER_BINDINGDESTROYED, Buffer{}))));
	uut->on_message_received(ScatterGather::Unsubscribe::create(Id{1}), *node1);
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_BINDINGDESTROYED | GATHER_FINISHED, Buffer{}), *node1);

	EXPECT_CALL(*leafA, send(Msg(ScatterGather::Gather::create(
		Id{555}, GATHER_BINDINGDESTROYED | GATHER_FINISHED, Buffer{}))));
	uut->on_message_received(ScatterGather::BindingRemoved::create(
		Id{1}), *leafB);
}

TEST_F(NodeTest, CloseGatheringConnectionsWhileScatterGatherOverLeaf)
{
	using namespace scatter_gather;
	Buffer buffer = prepare_scatter_gather_test();

	// run scatter-gather operation over leafA and close connections
	EXPECT_CALL(*node1, send(Msg(ScatterGather::Scatter::create(
		Id{104}, Id{1}, buffer))));
	EXPECT_CALL(*leafB, send(Msg(ScatterGather::Scatter::create(
		Id{61}, Id{1}, buffer))));
	uut->on_message_received(ScatterGather::Scatter::create(
		Id{1}, Id{555}, buffer), *leafA);

	EXPECT_CALL(*leafA, send(Msg(ScatterGather::Gather::create(
		Id{555}, GATHER_CONNECTIONLOST, Buffer{}))));
	uut->on_connection_destroyed(*node1);

	EXPECT_CALL(*leafA, send(Msg(ScatterGather::Gather::create(
		Id{555}, GATHER_CONNECTIONLOST | GATHER_FINISHED, Buffer{}))));
	uut->on_connection_destroyed(*leafB);
}

TEST_F(NodeTest, RemoveScatteringTerminalWhileScatterGatherOverLeaf)
{
	using namespace scatter_gather;
	Buffer buffer = prepare_scatter_gather_test();

	// run scatter-gather operation over leafA and remove the terminal which
	// initiated the scatter-gather operation
	EXPECT_CALL(*node1, send(Msg(ScatterGather::Scatter::create(
		Id{104}, Id{1}, buffer))));
	EXPECT_CALL(*leafB, send(Msg(ScatterGather::Scatter::create(
		Id{61}, Id{1}, buffer))));
	uut->on_message_received(ScatterGather::Scatter::create(
		Id{1}, Id{555}, buffer), *leafA);

	uut->on_message_received(ScatterGather::TerminalRemoved::create(
		Id{1}), *leafA);

	EXPECT_CALL(*leafA, send(MsgType(ScatterGather::Gather{})))
		.Times(0);
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_FINISHED, buffer), *node1);
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_FINISHED, buffer), *leafB);
}

TEST_F(NodeTest, CloseScatteringConnectionWhileScatterGatherOverLeaf)
{
	using namespace scatter_gather;
	Buffer buffer = prepare_scatter_gather_test();

	// run scatter-gather operation over leafA and close the connection to
	// leafA
	EXPECT_CALL(*node1, send(Msg(ScatterGather::Scatter::create(
		Id{104}, Id{1}, buffer))));
	EXPECT_CALL(*leafB, send(Msg(ScatterGather::Scatter::create(
		Id{61}, Id{1}, buffer))));
	uut->on_message_received(ScatterGather::Scatter::create(
		Id{1}, Id{555}, buffer), *leafA);

	uut->on_connection_destroyed(*leafA);

	EXPECT_CALL(*leafA, send(_))
		.Times(0);
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_FINISHED, buffer), *node1);
	uut->on_message_received(ScatterGather::Gather::create(
		Id{1}, GATHER_FINISHED, buffer), *leafB);
}

TEST_F(NodeTest, GetKnownTerminals)
{
    uut->on_connection_destroyed(*leafB);
    uut->on_connection_destroyed(*node1);
    uut->on_connection_destroyed(*node2);

    ignore_messages<DeafMute::TerminalMapping,
        PublishSubscribe::TerminalMapping, ScatterGather::TerminalMapping>();

    // create some terminals
    base::Identifier a{100u, "Ann", false};
    uut->on_message_received(DeafMute::TerminalDescription::create(
        a, Id{1}), *leafA);

    base::Identifier b{1234567u, "Lucy", false};
    uut->on_message_received(DeafMute::TerminalDescription::create(
        b, Id{2}), *leafA);

    base::Identifier c{56u, "Tom", false};
    uut->on_message_received(PublishSubscribe::TerminalDescription::create(
        c, Id{3}), *leafA);

    base::Identifier d{654u, "Campbell", false};
    uut->on_message_received(ScatterGather::TerminalDescription::create(
        d, Id{4}), *leafA);

    base::Identifier e{777u, "Rose", true};
    uut->on_message_received(ScatterGather::TerminalDescription::create(
        e, Id{5}), *leafA);

    // get the known terminals
    auto v  = uut->get_known_terminals();
    auto dm = std::vector<base::Identifier>{a, b};
    EXPECT_EQ(dm, v.deafMute);
    auto ps = std::vector<base::Identifier>{c};
    EXPECT_EQ(ps, v.publishSubscribe);
    auto sg = std::vector<base::Identifier>{d};
    EXPECT_EQ(sg, v.scatterGather);
}

TEST_F(NodeTest, AsyncAwaitKnownTerminalsChange)
{
    uut->on_connection_destroyed(*leafB);
    uut->on_connection_destroyed(*node1);
    uut->on_connection_destroyed(*node2);

    ignore_messages<PublishSubscribe::TerminalMapping,
        PublishSubscribe::TerminalRemovedAck>();

    base::Identifier identifier{123u, "Peanuts", false};

    int calls = 0;
    auto fn = [&](const api::Exception& e,
        Node::known_terminal_change_info info) {
            EXPECT_EQ(YOGI_OK, e.error_code());
            EXPECT_EQ(YOGI_TM_PUBLISHSUBSCRIBE, info.type);
            EXPECT_EQ(identifier, info.identifier);

            if (calls == 0) {
                EXPECT_TRUE(info.added);
            }
            else {
                EXPECT_FALSE(info.added);
            }

            ++calls;
    };

    uut->async_await_known_terminals_change(fn);
    uut->on_message_received(PublishSubscribe::TerminalDescription::create(
        base::Identifier{444u, "Invisible", true}, Id{3}), *leafA);
    uut->on_message_received(PublishSubscribe::TerminalDescription::create(
        identifier, Id{5}), *leafA);
    EXPECT_EQ(1, calls);

    uut->async_await_known_terminals_change(fn);
    uut->on_message_received(PublishSubscribe::TerminalRemoved::create(Id{2}),
        *leafA);
    uut->on_message_received(PublishSubscribe::TerminalRemoved::create(Id{1}),
        *leafA);
    EXPECT_EQ(2, calls);
}

TEST_F(NodeTest, CancelAwaitKnownTerminalsChange)
{
    int calls = 0;
    uut->async_await_known_terminals_change(
        [&](const api::Exception& e, Node::known_terminal_change_info info) {
            EXPECT_EQ(YOGI_ERR_CANCELED, e.error_code());
            ++calls;
        }
    );

    uut->cancel_await_known_terminals_change();
    EXPECT_EQ(1, calls);
}
