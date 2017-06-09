#include <gtest/gtest.h>

#include "../yogi/scheduler.hpp"
#include "../yogi/leaf.hpp"
#include "../yogi/terminals.hpp"
#include "../yogi/binding.hpp"
#include "../yogi/connection.hpp"
#include "../yogi/errors.hpp"
using namespace yogi;
using namespace yogi::errors;

#include <atomic>


struct RawTerminalsTest : public testing::Test
{
    Scheduler                        scheduler;
    Leaf                             leafA;
    Leaf                             leafB;
    std::unique_ptr<LocalConnection> conn;

    struct Terminals {
        RawDeafMuteTerminal               dm;
        RawPublishSubscribeTerminal       ps;
        RawCachedPublishSubscribeTerminal cps;
        RawScatterGatherTerminal          sg;

        RawProducerTerminal       pcProducer;
        RawConsumerTerminal       pcConsumer;
        RawCachedProducerTerminal cpcProducer;
        RawCachedConsumerTerminal cpcConsumer;
        RawMasterTerminal         msMaster;
        RawSlaveTerminal          msSlave;
        RawCachedMasterTerminal   cmsMaster;
        RawCachedSlaveTerminal    cmsSlave;
        RawServiceTerminal        scService;
        RawClientTerminal         scClient;

        Terminals(Leaf& leaf)
        : dm         (leaf, "DM Terminal",  Signature(123))
        , ps         (leaf, "PS Terminal",  Signature(123))
        , cps        (leaf, "CPS Terminal", Signature(123))
        , sg         (leaf, "SG Terminal",  Signature(123))
        , pcProducer (leaf, "PC Terminal",  Signature(123))
        , pcConsumer (leaf, "PC Terminal",  Signature(123))
        , cpcProducer(leaf, "CPC Terminal", Signature(123))
        , cpcConsumer(leaf, "CPC Terminal", Signature(123))
        , msMaster   (leaf, "MS Terminal",  Signature(123))
        , msSlave    (leaf, "MS Terminal",  Signature(123))
        , cmsMaster  (leaf, "CMS Terminal", Signature(123))
        , cmsSlave   (leaf, "CMS Terminal", Signature(123))
        , scService  (leaf, "SC Terminal",  Signature(123))
        , scClient   (leaf, "SC Terminal",  Signature(123))
        {
        }
    };

    Terminals terminalsA;
    Terminals terminalsB;

    struct Bindings {
        Binding dm;
        Binding ps;
        Binding cps;
        Binding sg;

        Bindings(Terminals& terminals)
        : dm( terminals.dm,  terminals.dm .name())
        , ps (terminals.ps,  terminals.ps .name())
        , cps(terminals.cps, terminals.cps.name())
        , sg (terminals.sg,  terminals.sg .name())
        {
        }
    };

    Bindings bindings;

    RawTerminalsTest()
    : leafA(scheduler)
    , leafB(scheduler)
    , terminalsA(leafA)
    , terminalsB(leafB)
    , bindings(terminalsA)
    {
    }

    void connect(Binder& binder)
    {
        EXPECT_EQ(RELEASED, binder.get_binding_state());
        conn = std::make_unique<LocalConnection>(leafA, leafB);
        while (binder.get_binding_state() == RELEASED);
        EXPECT_EQ(ESTABLISHED, binder.get_binding_state());
    }

    void connect(Binder& binder, Subscribable& subscribable)
    {
        connect(binder);
        while (subscribable.get_subscription_state() == UNSUBSCRIBED);
        EXPECT_EQ(SUBSCRIBED, subscribable.get_subscription_state());
    }
};

TEST_F(RawTerminalsTest, Type)
{
    EXPECT_EQ(terminal_type::DEAF_MUTE,                terminalsA.dm.type());
    EXPECT_EQ(terminal_type::PUBLISH_SUBSCRIBE,        terminalsA.ps.type());
    EXPECT_EQ(terminal_type::SCATTER_GATHER,           terminalsA.sg.type());
    EXPECT_EQ(terminal_type::CACHED_PUBLISH_SUBSCRIBE, terminalsA.cps.type());
    EXPECT_EQ(terminal_type::PRODUCER,                 terminalsA.pcProducer.type());
    EXPECT_EQ(terminal_type::CONSUMER,                 terminalsA.pcConsumer.type());
    EXPECT_EQ(terminal_type::CACHED_PRODUCER,          terminalsA.cpcProducer.type());
    EXPECT_EQ(terminal_type::CACHED_CONSUMER,          terminalsA.cpcConsumer.type());
    EXPECT_EQ(terminal_type::MASTER,                   terminalsA.msMaster.type());
    EXPECT_EQ(terminal_type::SLAVE,                    terminalsA.msSlave.type());
    EXPECT_EQ(terminal_type::CACHED_MASTER,            terminalsA.cmsMaster.type());
    EXPECT_EQ(terminal_type::CACHED_SLAVE,             terminalsA.cmsSlave.type());
    EXPECT_EQ(terminal_type::SERVICE,                  terminalsA.scService.type());
    EXPECT_EQ(terminal_type::CLIENT,                   terminalsA.scClient.type());
}

TEST_F(RawTerminalsTest, DeafMuteTerminal)
{
    connect(bindings.dm);
}

TEST_F(RawTerminalsTest, PublishSubscribeTerminal)
{
    std::vector<char> msg{12, 34};

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.ps.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.ps.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.ps.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.ps.cancel_receive_message();
    while (!called);

    // successfully receive message
    connect(bindings.ps, terminalsB.ps);
    called = false;
    terminalsA.ps.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.ps.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.ps.try_publish(msg));
}

TEST_F(RawTerminalsTest, CachedPublishSubscribeTerminal)
{
    std::vector<char> msg{12, 34};

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.cps.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cps.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.cps.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cps.cancel_receive_message();
    while (!called);

    // successfully receive cached message
    called = false;
    terminalsA.cps.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    connect(bindings.cps, terminalsB.cps);
    while (!called);

    // successfully receive non-cached message
    called = false;
    terminalsA.cps.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cps.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.cps.try_publish(msg));

    // get cached message
    auto data = terminalsA.cps.get_cached_message();
    EXPECT_EQ(2u, data.size());
    EXPECT_EQ(12, data[0]);
    EXPECT_EQ(34, data[1]);
}

TEST_F(RawTerminalsTest, ScatterGatherTerminal)
{
    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.sg.async_receive_scattered_message([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.sg.cancel_receive_scattered_message();
    while (!called);

    // ignore message
    connect(bindings.sg, terminalsB.sg);

    terminalsA.sg.async_receive_scattered_message([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, msg.data().size());
        EXPECT_EQ(12, msg.data()[0]);
        EXPECT_EQ(34, msg.data()[1]);
        msg.ignore();
    });

    std::vector<char> msg{12, 34};

    called = false;
    auto operation = terminalsB.sg.async_scatter_gather(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_TRUE(msg.flags() & IGNORED);
        called = true;
        return CONTINUE;
    });

    while (!called);

    // respond to message
    terminalsA.sg.async_receive_scattered_message([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, msg.data().size());
        EXPECT_EQ(12, msg.data()[0]);
        EXPECT_EQ(34, msg.data()[1]);

        std::vector<char> response{56, 78};
        msg.respond(response);
    });

    called = false;
    operation = terminalsB.sg.async_scatter_gather(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_TRUE(msg.flags() & FINISHED);
        EXPECT_EQ(2u, msg.data().size());
        EXPECT_EQ(56, msg.data()[0]);
        EXPECT_EQ(78, msg.data()[1]);
        called = true;
        return CONTINUE;
    });

    while (!called);

    // cancel operation
    terminalsA.sg.async_receive_scattered_message([&](auto& res, auto&& msg) {
    });

    operation = terminalsB.sg.async_scatter_gather(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
        return STOP;
    });

    operation.cancel();
    while (!called);
}

TEST_F(RawTerminalsTest, ProducerConsumerTerminals)
{
    std::vector<char> msg{12, 34};

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.pcProducer.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.pcProducer.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.pcConsumer.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.pcConsumer.cancel_receive_message();
    while (!called);

    // successfully receive message
    connect(terminalsA.pcConsumer, terminalsB.pcProducer);
    called = false;
    terminalsA.pcConsumer.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.pcProducer.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.pcProducer.try_publish(msg));
}

TEST_F(RawTerminalsTest, CachedProducerConsumerTerminals)
{
    std::vector<char> msg{12, 34};

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.cpcProducer.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cpcProducer.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.cpcConsumer.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cpcConsumer.cancel_receive_message();
    while (!called);

    // successfully receive cached message
    called = false;
    terminalsA.cpcConsumer.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    connect(terminalsA.cpcConsumer, terminalsB.cpcProducer);
    while (!called);

    // successfully receive non-cached message
    called = false;
    terminalsA.cpcConsumer.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cpcProducer.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.cpcProducer.try_publish(msg));

    // get cached message
    auto data = terminalsA.cpcConsumer.get_cached_message();
    EXPECT_EQ(2u, data.size());
    EXPECT_EQ(12, data[0]);
    EXPECT_EQ(34, data[1]);
}

TEST_F(RawTerminalsTest, MasterSlaveTerminals)
{
    std::vector<char> msg{12, 34};

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.msMaster.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.msMaster.try_publish(msg));

    EXPECT_THROW(terminalsB.msSlave.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.msSlave.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.msSlave.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.msSlave.cancel_receive_message();
    while (!called);

    called = false;
    terminalsA.msMaster.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.msMaster.cancel_receive_message();
    while (!called);

    // successfully receive message
    connect(terminalsA.msSlave, terminalsB.msMaster);
    while (terminalsB.msMaster.get_binding_state() == RELEASED);

    called = false;
    terminalsA.msSlave.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.msMaster.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.msMaster.try_publish(msg));

    called = false;
    terminalsA.msMaster.async_receive_message([&](auto& res, auto data) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.msSlave.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.msSlave.try_publish(msg));
}

TEST_F(RawTerminalsTest, CachedMasterSlaveTerminal)
{
    std::vector<char> msg{12, 34};

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.cmsMaster.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cmsMaster.try_publish(msg));

    EXPECT_THROW(terminalsB.cmsSlave.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cmsSlave.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.cmsSlave.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cmsSlave.cancel_receive_message();
    while (!called);

    called = false;
    terminalsA.cmsMaster.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cmsMaster.cancel_receive_message();
    while (!called);

    // successfully receive cached message
    called = false;
    terminalsA.cmsSlave.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    std::atomic<bool> called2{false};
    terminalsA.cmsMaster.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called2 = true;
    });

    connect(terminalsA.cmsSlave, terminalsB.cmsMaster);
    while (terminalsB.cmsMaster.get_binding_state() == RELEASED);
    while (!called || !called2);

    // successfully receive non-cached message published by the master
    called = false;
    terminalsA.cmsSlave.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cmsMaster.publish(msg));
    while (!called);

    // successfully receive non-cached message published by the slave
    called = false;
    terminalsA.cmsMaster.async_receive_message([&](auto& res, auto data, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, data.size());
        EXPECT_EQ(12, data[0]);
        EXPECT_EQ(34, data[1]);
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cmsSlave.publish(msg));
    while (!called);

    // check try_publish()
    EXPECT_TRUE(terminalsB.cmsMaster.try_publish(msg));
    EXPECT_TRUE(terminalsB.cmsSlave.try_publish(msg));

    // get cached message
    auto data = terminalsA.cmsMaster.get_cached_message();
    EXPECT_EQ(2u, data.size());
    EXPECT_EQ(12, data[0]);
    EXPECT_EQ(34, data[1]);

    data = terminalsA.cmsSlave.get_cached_message();
    EXPECT_EQ(2u, data.size());
    EXPECT_EQ(12, data[0]);
    EXPECT_EQ(34, data[1]);
}

TEST_F(RawTerminalsTest, ServiceClientTerminals)
{
    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.scService.async_receive_request([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.scService.cancel_receive_request();
    while (!called);

    // ignore message
    connect(terminalsA.scService, terminalsB.scClient);

    terminalsA.scService.async_receive_request([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, msg.data().size());
        EXPECT_EQ(12, msg.data()[0]);
        EXPECT_EQ(34, msg.data()[1]);
        msg.ignore();
    });

    std::vector<char> msg{12, 34};

    called = false;
    auto operation = terminalsB.scClient.async_request(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_TRUE(msg.flags() & IGNORED);
        called = true;
        return CONTINUE;
    });

    while (!called);

    // respond to message
    terminalsA.scService.async_receive_request([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(2u, msg.data().size());
        EXPECT_EQ(12, msg.data()[0]);
        EXPECT_EQ(34, msg.data()[1]);

        std::vector<char> response{56, 78};
        msg.respond(response);
    });

    called = false;
    operation = terminalsB.scClient.async_request(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_TRUE(msg.flags() & FINISHED);
        EXPECT_EQ(2u, msg.data().size());
        EXPECT_EQ(56, msg.data()[0]);
        EXPECT_EQ(78, msg.data()[1]);
        called = true;
        return CONTINUE;
    });

    while (!called);

    // cancel operation
    terminalsA.scService.async_receive_request([&](auto& res, auto&& msg) {
    });

    operation = terminalsB.scClient.async_request(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
        return STOP;
    });

    operation.cancel();
    while (!called);
}

TEST_F(RawTerminalsTest, MakeRawTerminal)
{
    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::DEAF_MUTE, "T", Signature(0));
    EXPECT_EQ(terminal_type::DEAF_MUTE, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::PUBLISH_SUBSCRIBE, "T", Signature(0));
    EXPECT_EQ(terminal_type::PUBLISH_SUBSCRIBE, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::SCATTER_GATHER, "T", Signature(0));
    EXPECT_EQ(terminal_type::SCATTER_GATHER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CACHED_PUBLISH_SUBSCRIBE, "T", Signature(0));
    EXPECT_EQ(terminal_type::CACHED_PUBLISH_SUBSCRIBE, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::PRODUCER, "T", Signature(0));
    EXPECT_EQ(terminal_type::PRODUCER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CONSUMER, "T", Signature(0));
    EXPECT_EQ(terminal_type::CONSUMER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CACHED_PRODUCER, "T", Signature(0));
    EXPECT_EQ(terminal_type::CACHED_PRODUCER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CACHED_CONSUMER, "T", Signature(0));
    EXPECT_EQ(terminal_type::CACHED_CONSUMER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::MASTER, "T", Signature(0));
    EXPECT_EQ(terminal_type::MASTER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::SLAVE, "T", Signature(0));
    EXPECT_EQ(terminal_type::SLAVE, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CACHED_MASTER, "T", Signature(0));
    EXPECT_EQ(terminal_type::CACHED_MASTER, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CACHED_SLAVE, "T", Signature(0));
    EXPECT_EQ(terminal_type::CACHED_SLAVE, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::SERVICE, "T", Signature(0));
    EXPECT_EQ(terminal_type::SERVICE, tm->type());
    }}

    {{
    auto tm = Terminal::make_raw_terminal(leafA, terminal_type::CLIENT, "T", Signature(0));
    EXPECT_EQ(terminal_type::CLIENT, tm->type());
    }}
}
