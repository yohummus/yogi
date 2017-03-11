#include <gtest/gtest.h>

#include "../yogi_cpp/scheduler.hpp"
#include "../yogi_cpp/leaf.hpp"
#include "../yogi_cpp/terminals.hpp"
#include "../yogi_cpp/binding.hpp"
#include "../yogi_cpp/connection.hpp"
#include "../yogi_cpp/errors.hpp"
using namespace yogi;
using namespace yogi::errors;

#include "proto/yogi_0000d007.h"

#include <atomic>


struct TerminalsTest : public testing::Test
{
    Scheduler                        scheduler;
    Leaf                             leafA;
    Leaf                             leafB;
    std::unique_ptr<LocalConnection> conn;

    struct Terminals {
        DeafMuteTerminal              <yogi_0000d007> dm;
        PublishSubscribeTerminal      <yogi_0000d007> ps;
        CachedPublishSubscribeTerminal<yogi_0000d007> cps;
        ScatterGatherTerminal         <yogi_0000d007> sg;

        ProducerTerminal      <yogi_0000d007> pcProducer;
        ConsumerTerminal      <yogi_0000d007> pcConsumer;
        CachedProducerTerminal<yogi_0000d007> cpcProducer;
        CachedConsumerTerminal<yogi_0000d007> cpcConsumer;
        MasterTerminal        <yogi_0000d007> msMaster;
        SlaveTerminal         <yogi_0000d007> msSlave;
        CachedMasterTerminal  <yogi_0000d007> cmsMaster;
        CachedSlaveTerminal   <yogi_0000d007> cmsSlave;
        ServiceTerminal       <yogi_0000d007> scService;
        ClientTerminal        <yogi_0000d007> scClient;

        Terminals(Leaf& leaf)
        : dm         (leaf, "DM Terminal")
        , ps         (leaf, "PS Terminal")
        , cps        (leaf, "CPS Terminal")
        , sg         (leaf, "SG Terminal")
        , pcProducer (leaf, "PC Terminal")
        , pcConsumer (leaf, "PC Terminal")
        , cpcProducer(leaf, "CPC Terminal")
        , cpcConsumer(leaf, "CPC Terminal")
        , msMaster   (leaf, "MS Terminal")
        , msSlave    (leaf, "MS Terminal")
        , cmsMaster  (leaf, "CMS Terminal")
        , cmsSlave   (leaf, "CMS Terminal")
        , scService  (leaf, "SC Terminal")
        , scClient   (leaf, "SC Terminal")
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

    TerminalsTest()
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

TEST_F(TerminalsTest, Type)
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

TEST_F(TerminalsTest, DeafMuteTerminal)
{
    connect(bindings.dm);
}

TEST_F(TerminalsTest, PublishSubscribeTerminal)
{
    auto msg = terminalsB.ps.make_message();
    msg.set_value(123);

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.ps.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.ps.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.ps.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.ps.cancel_receive_message();
    while (!called);

    // successfully receive message
    connect(bindings.ps, terminalsB.ps);
    called = false;
    terminalsA.ps.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.ps.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.ps.try_publish(msg));
}

TEST_F(TerminalsTest, CachedPublishSubscribeTerminal)
{
    auto msg = terminalsB.cps.make_message();
    msg.set_value(123);

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.cps.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cps.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.cps.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cps.cancel_receive_message();
    while (!called);

    // successfully receive cached message
    called = false;
    terminalsA.cps.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    connect(bindings.cps, terminalsB.cps);
    while (!called);

    // successfully receive non-cached message
    called = false;
    terminalsA.cps.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cps.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.cps.try_publish(msg));

    // get cached message
    msg = terminalsA.cps.get_cached_message();
    EXPECT_EQ(123, msg.value());
}

TEST_F(TerminalsTest, ScatterGatherTerminal)
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
        EXPECT_EQ("test", msg.message().value());
        msg.ignore();
    });

    auto msg = terminalsB.sg.make_scatter_message();
    msg.set_value("test");

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
        EXPECT_EQ("test", msg.message().value());

        auto response = terminalsA.sg.make_gather_message();
        response.set_value(123);
        msg.respond(response);
    });

    called = false;
    operation = terminalsB.sg.async_scatter_gather(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_TRUE(msg.flags() & FINISHED);
        EXPECT_EQ(123, msg.message().value());
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

TEST_F(TerminalsTest, ProducerConsumerTerminals)
{
    auto msg = terminalsB.pcProducer.make_message();
    msg.set_value(123);

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.pcProducer.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.pcProducer.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.pcConsumer.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.pcConsumer.cancel_receive_message();
    while (!called);

    // successfully receive message
    connect(terminalsA.pcConsumer, terminalsB.pcProducer);
    called = false;
    terminalsA.pcConsumer.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.pcProducer.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.pcProducer.try_publish(msg));
}

TEST_F(TerminalsTest, CachedProducerConsumerTerminals)
{
    auto msg = terminalsB.cpcProducer.make_message();
    msg.set_value(123);

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.cpcProducer.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cpcProducer.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.cpcConsumer.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cpcConsumer.cancel_receive_message();
    while (!called);

    // successfully receive cached message
    called = false;
    terminalsA.cpcConsumer.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    connect(terminalsA.cpcConsumer, terminalsB.cpcProducer);
    while (!called);

    // successfully receive non-cached message
    called = false;
    terminalsA.cpcConsumer.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cpcProducer.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.cpcProducer.try_publish(msg));

    // get cached message
    msg = terminalsA.cpcConsumer.get_cached_message();
    EXPECT_EQ(123, msg.value());
}

TEST_F(TerminalsTest, MasterSlaveTerminals)
{
    auto msg = terminalsB.msMaster.make_message();
    msg.set_value(123);

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.msMaster.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.msMaster.try_publish(msg));

    EXPECT_THROW(terminalsB.msSlave.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.msSlave.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.msSlave.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.msSlave.cancel_receive_message();
    while (!called);

    called = false;
    terminalsA.msMaster.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.msMaster.cancel_receive_message();
    while (!called);

    // successfully receive message
    connect(terminalsA.msSlave, terminalsB.msMaster);
    while (terminalsB.msMaster.get_binding_state() == RELEASED);

    called = false;
    terminalsA.msSlave.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.msMaster.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.msMaster.try_publish(msg));

    called = false;
    terminalsA.msMaster.async_receive_message([&](auto& res, auto msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.msSlave.publish(msg));
    while (!called);

    EXPECT_TRUE(terminalsB.msSlave.try_publish(msg));
}

TEST_F(TerminalsTest, CachedMasterSlaveTerminal)
{
    auto msg = terminalsB.cmsMaster.make_message();
    msg.set_value(123);

    // publish without the terminal being bound
    EXPECT_THROW(terminalsB.cmsMaster.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cmsMaster.try_publish(msg));

    EXPECT_THROW(terminalsB.cmsSlave.publish(msg), Failure);
    EXPECT_FALSE(terminalsB.cmsSlave.try_publish(msg));

    // cancel receive message
    std::atomic<bool> called{false};
    terminalsA.cmsSlave.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cmsSlave.cancel_receive_message();
    while (!called);

    called = false;
    terminalsA.cmsMaster.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_EQ(res, Canceled());
        called = true;
    });

    terminalsA.cmsMaster.cancel_receive_message();
    while (!called);

    // successfully receive cached message
    called = false;
    terminalsA.cmsSlave.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    std::atomic<bool> called2{false};
    terminalsA.cmsMaster.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_TRUE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called2 = true;
    });

    connect(terminalsA.cmsSlave, terminalsB.cmsMaster);
    while (terminalsB.cmsMaster.get_binding_state() == RELEASED);
    while (!called || !called2);

    // successfully receive non-cached message published by the master
    called = false;
    terminalsA.cmsSlave.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cmsMaster.publish(msg));
    while (!called);

    // successfully receive non-cached message published by the slave
    called = false;
    terminalsA.cmsMaster.async_receive_message([&](auto& res, auto msg, auto cached) {
        EXPECT_FALSE(cached);
        EXPECT_EQ(res, Success());
        EXPECT_EQ(123, msg.value());
        called = true;
    });

    EXPECT_NO_THROW(terminalsB.cmsSlave.publish(msg));
    while (!called);

    // check try_publish()
    EXPECT_TRUE(terminalsB.cmsMaster.try_publish(msg));
    EXPECT_TRUE(terminalsB.cmsSlave.try_publish(msg));

    // get cached message
    msg = terminalsA.cmsMaster.get_cached_message();
    EXPECT_EQ(123, msg.value());

    msg = terminalsA.cmsSlave.get_cached_message();
    EXPECT_EQ(123, msg.value());
}

TEST_F(TerminalsTest, ServiceClientTerminals)
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
        EXPECT_EQ("test", msg.message().value());
        msg.ignore();
    });

    auto msg = terminalsB.scClient.make_request_message();
    msg.set_value("test");

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
        EXPECT_EQ("test", msg.message().value());

        auto response = terminalsA.scService.make_response_message();
        response.set_value(123);
        msg.respond(response);
    });

    called = false;
    operation = terminalsB.scClient.async_request(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_TRUE(msg.flags() & FINISHED);
        EXPECT_EQ(123, msg.message().value());
        called = true;
        return CONTINUE;
    });

    while (!called);

    // cancel operation
    terminalsA.scService.async_receive_request([&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ("test", msg.message().value());
    });

    operation = terminalsB.scClient.async_request(msg, [&](auto& res, auto&& msg) {
        EXPECT_EQ(res, Canceled());
        called = true;
        return STOP;
    });

    operation.cancel();
    while (!called);
}
