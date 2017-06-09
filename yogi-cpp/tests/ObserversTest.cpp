#include <gtest/gtest.h>

#include "../yogi/scheduler.hpp"
#include "../yogi/leaf.hpp"
#include "../yogi/binding.hpp"
#include "../yogi/terminals.hpp"
#include "../yogi/observers.hpp"
#include "../yogi/connection.hpp"
using namespace yogi;

#include "proto/yogi_0000d007.h"

#include <atomic>


struct ObserversTest : public testing::Test
{
    Scheduler                              scheduler;
    Leaf                                   leafA;
    Leaf                                   leafB;
    ProducerTerminal<yogi_0000d007>       producer;
    ConsumerTerminal<yogi_0000d007>       consumer;
    CachedProducerTerminal<yogi_0000d007> cachedProducer;
    CachedConsumerTerminal<yogi_0000d007> cachedConsumer;
    ScatterGatherTerminal<yogi_0000d007>  scatterGatherA;
    ScatterGatherTerminal<yogi_0000d007>  scatterGatherB;
    Binding                                scatterGatherBinding;
    ServiceTerminal<yogi_0000d007>        service;
    ClientTerminal<yogi_0000d007>         client;
    RawProducerTerminal                    rawProducer;
    RawConsumerTerminal                    rawConsumer;
    RawCachedProducerTerminal              rawCachedProducer;
    RawCachedConsumerTerminal              rawCachedConsumer;
    RawScatterGatherTerminal               rawScatterGatherA;
    RawScatterGatherTerminal               rawScatterGatherB;
    Binding                                rawScatterGatherBinding;
    RawServiceTerminal                     rawService;
    RawClientTerminal                      rawClient;

    ObserversTest()
    : leafA                  (scheduler)
    , leafB                  (scheduler)
    , producer               (leafA, "T")
    , consumer               (leafB, "T")
    , cachedProducer         (leafA, "T")
    , cachedConsumer         (leafB, "T")
    , scatterGatherA         (leafA, "T")
    , scatterGatherB         (leafB, "T")
    , scatterGatherBinding   (scatterGatherA, "T")
    , service                (leafA, "T")
    , client                 (leafB, "T")
    , rawProducer            (leafA, "T", Signature(0))
    , rawConsumer            (leafB, "T", Signature(0))
    , rawCachedProducer      (leafA, "T", Signature(0))
    , rawCachedConsumer      (leafB, "T", Signature(0))
    , rawScatterGatherA      (leafA, "T", Signature(0))
    , rawScatterGatherB      (leafB, "T", Signature(0))
    , rawScatterGatherBinding(rawScatterGatherA, "T")
    , rawService             (leafA, "T", Signature(0))
    , rawClient              (leafB, "T", Signature(0))
    {
    }
};

TEST_F(ObserversTest, BindingObserver)
{
    BindingObserver observer(consumer);

    int i = 0;
    auto id = observer.add([&](binding_state state) {
        switch (i++) {
        case 0: EXPECT_EQ(RELEASED,    state); break;
        case 1: EXPECT_EQ(ESTABLISHED, state); break;
        case 2: EXPECT_EQ(RELEASED,    state); break;
        }
    });

    int j = 0;
    observer.add([&](binding_state state) {
        switch (j++) {
        case 0: EXPECT_EQ(RELEASED,    state); break;
        case 1: EXPECT_EQ(ESTABLISHED, state); break;
        case 2: EXPECT_EQ(RELEASED,    state); break;
        case 3: EXPECT_EQ(ESTABLISHED, state); break;
        }
    });

    EXPECT_EQ(0, i);
    EXPECT_EQ(0, j);

    observer.start();

    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (consumer.get_binding_state() == RELEASED);
    conn.reset();
    while (consumer.get_binding_state() == ESTABLISHED);

    observer.remove(id);
    conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (consumer.get_binding_state() == RELEASED);

    observer.stop();
    conn.reset();
    while (consumer.get_binding_state() == ESTABLISHED);

    EXPECT_EQ(3, i);
    EXPECT_EQ(4, j);
}

TEST_F(ObserversTest, SubscriptionObserver)
{
    SubscriptionObserver observer(producer);

    int i = 0;
    auto id = observer.add([&](subscription_state state) {
        switch (i++) {
        case 0: EXPECT_EQ(UNSUBSCRIBED, state); break;
        case 1: EXPECT_EQ(SUBSCRIBED,   state); break;
        case 2: EXPECT_EQ(UNSUBSCRIBED, state); break;
        }
    });

    int j = 0;
    observer.add([&](subscription_state state) {
        switch (j++) {
        case 0: EXPECT_EQ(UNSUBSCRIBED, state); break;
        case 1: EXPECT_EQ(SUBSCRIBED,   state); break;
        case 2: EXPECT_EQ(UNSUBSCRIBED, state); break;
        case 3: EXPECT_EQ(SUBSCRIBED,   state); break;
        }
    });

    EXPECT_EQ(0, i);
    EXPECT_EQ(0, j);

    observer.start();

    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (producer.get_subscription_state() == UNSUBSCRIBED);
    conn.reset();
    while (producer.get_subscription_state() == SUBSCRIBED);

    observer.remove(id);
    conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (producer.get_subscription_state() == UNSUBSCRIBED);

    observer.stop();
    conn.reset();
    while (producer.get_subscription_state() == SUBSCRIBED);

    EXPECT_EQ(3, i);
    EXPECT_EQ(4, j);
}

TEST_F(ObserversTest, AllTerminalTypesCompile)
{
    {{
    PublishSubscribeTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    CachedPublishSubscribeTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    ScatterGatherTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    ConsumerTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    CachedConsumerTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    MasterTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    SlaveTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    CachedMasterTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    CachedSlaveTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}

    {{
    ServiceTerminal<yogi_0000d007> tm(leafA, "A");
    MessageObserver<decltype(tm)> observer(tm);
    }}
}

TEST_F(ObserversTest, MessageObserverPublishNonCached)
{
    MessageObserver<decltype(consumer)> observer(consumer);

    std::atomic<int> i{0};
    auto id = observer.add([&](auto& msg) {
        ++i;
        EXPECT_EQ(123, msg.value());
    });

    observer.add([&](auto& msg, auto cached) {
        ++i;
        EXPECT_EQ(123, msg.value());
        EXPECT_FALSE(cached);
    });

    EXPECT_EQ(0, i);

    observer.start();
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (consumer.get_binding_state() == RELEASED || producer.get_subscription_state() == UNSUBSCRIBED);

    auto msg = producer.make_message();
    msg.set_value(123);
    while (!producer.try_publish(msg));
    while (i < 2);
    EXPECT_EQ(2, i);

    observer.remove(id);
    while (!producer.try_publish(msg));
    while (i < 3);
    EXPECT_EQ(3, i);

    observer.stop();
    while (!producer.try_publish(msg));
    EXPECT_EQ(3, i);
}

TEST_F(ObserversTest, RawMessageObserverPublishNonCached)
{
    MessageObserver<decltype(rawConsumer)> observer(rawConsumer);

    std::atomic<int> i{0};
    auto id = observer.add([&](auto& data) {
        ++i;
        EXPECT_EQ(std::vector<char>{123}, data);
    });

    observer.add([&](auto& data, auto cached) {
        ++i;
        EXPECT_EQ(std::vector<char>{123}, data);
        EXPECT_FALSE(cached);
    });

    EXPECT_EQ(0, i);

    observer.start();
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (rawConsumer.get_binding_state() == RELEASED || rawProducer.get_subscription_state() == UNSUBSCRIBED);

    auto data = std::vector<char>{123};
    while (!rawProducer.try_publish(data));
    while (i < 2);
    EXPECT_EQ(2, i);

    observer.remove(id);
    while (!rawProducer.try_publish(data));
    while (i < 3);
    EXPECT_EQ(3, i);

    observer.stop();
    while (!rawProducer.try_publish(data));
    EXPECT_EQ(3, i);
}

TEST_F(ObserversTest, MessageObserverPublishCached)
{
    MessageObserver<decltype(cachedConsumer)> observer(cachedConsumer);

    std::atomic<int> i{0};
    auto id = observer.add([&](auto& msg) {
        ++i;
        EXPECT_EQ(123, msg.value());
    });

    observer.add([&](auto& msg, auto cached) {
        ++i;
        EXPECT_EQ(123, msg.value());

        if (i < 3) {
            EXPECT_TRUE(cached);
        }
        else {
            EXPECT_FALSE(cached);
        }
    });

    EXPECT_EQ(0, i);

    auto msg = cachedProducer.make_message();
    msg.set_value(123);
    cachedProducer.try_publish(msg);

    observer.start();
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (cachedConsumer.get_binding_state() == RELEASED || cachedProducer.get_subscription_state() == UNSUBSCRIBED);

    while (i < 2);
    EXPECT_EQ(2, i);

    observer.remove(id);
    while (!cachedProducer.try_publish(msg));
    while (i < 3);
    EXPECT_EQ(3, i);

    observer.stop();
    while (!cachedProducer.try_publish(msg));
    EXPECT_EQ(3, i);
}

TEST_F(ObserversTest, RawMessageObserverPublishCached)
{
    MessageObserver<decltype(rawCachedConsumer)> observer(rawCachedConsumer);

    std::atomic<int> i{0};
    auto id = observer.add([&](auto& data) {
        ++i;
        EXPECT_EQ(std::vector<char>{123}, data);
    });

    observer.add([&](auto& data, auto cached) {
        ++i;
        EXPECT_EQ(std::vector<char>{123}, data);

        if (i < 3) {
            EXPECT_TRUE(cached);
        }
        else {
            EXPECT_FALSE(cached);
        }
    });

    EXPECT_EQ(0, i);

    auto data = std::vector<char>{123};
    rawCachedProducer.try_publish(data);

    observer.start();
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (rawCachedConsumer.get_binding_state() == RELEASED || rawCachedProducer.get_subscription_state() == UNSUBSCRIBED);

    while (i < 2);
    EXPECT_EQ(2, i);

    observer.remove(id);
    while (!rawCachedProducer.try_publish(data));
    while (i < 3);
    EXPECT_EQ(3, i);

    observer.stop();
    while (!rawCachedProducer.try_publish(data));
    EXPECT_EQ(3, i);
}

TEST_F(ObserversTest, ScatterGatherTerminal)
{
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (scatterGatherBinding.get_binding_state() == RELEASED || scatterGatherB.get_subscription_state() == UNSUBSCRIBED);

    MessageObserver<decltype(scatterGatherA)> observer(scatterGatherA);

    std::atomic<int> i{0};
    auto msg = scatterGatherB.make_scatter_message();
    msg.set_value("Hello");
    scatterGatherB.async_scatter_gather(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & DEAF);
        ++i;
        return STOP;
    });

    while (i < 1);
    EXPECT_EQ(1, i);

    observer.start();

    scatterGatherB.async_scatter_gather(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 2);
    EXPECT_EQ(2, i);

    observer.set([&](auto&& request) {
        EXPECT_EQ("Hello", request.message().value());
        auto msg = scatterGatherA.make_gather_message();
        msg.set_value(123);
        request.respond(msg);
    });

    scatterGatherB.async_scatter_gather(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_EQ(123, response.message().value());
        ++i;
        return STOP;
    });

    while (i < 3);
    EXPECT_EQ(3, i);

    observer.clear();

    scatterGatherB.async_scatter_gather(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 4);
    EXPECT_EQ(4, i);
}

TEST_F(ObserversTest, RawScatterGatherTerminal)
{
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (rawScatterGatherBinding.get_binding_state() == RELEASED || rawScatterGatherB.get_subscription_state() == UNSUBSCRIBED);

    MessageObserver<decltype(rawScatterGatherA)> observer(rawScatterGatherA);

    std::atomic<int> i{0};
    auto data = std::vector<char>{69};
    rawScatterGatherB.async_scatter_gather(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & DEAF);
        ++i;
        return STOP;
    });

    while (i < 1);
    EXPECT_EQ(1, i);

    observer.start();

    rawScatterGatherB.async_scatter_gather(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 2);
    EXPECT_EQ(2, i);

    observer.set([&](auto&& request) {
        EXPECT_EQ(std::vector<char>{69}, request.data());
        auto data = std::vector<char>{123};
        request.respond(data);
    });

    rawScatterGatherB.async_scatter_gather(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_EQ(std::vector<char>{123}, response.data());
        ++i;
        return STOP;
    });

    while (i < 3);
    EXPECT_EQ(3, i);

    observer.clear();

    rawScatterGatherB.async_scatter_gather(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 4);
    EXPECT_EQ(4, i);
}

TEST_F(ObserversTest, ServiceTerminal)
{
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (service.get_binding_state() == RELEASED || client.get_subscription_state() == UNSUBSCRIBED);

    MessageObserver<decltype(service)> observer(service);

    std::atomic<int> i{0};
    auto msg = client.make_request_message();
    msg.set_value("Hello");
    client.async_request(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & DEAF);
        ++i;
        return STOP;
    });

    while (i < 1);
    EXPECT_EQ(1, i);

    observer.start();

    client.async_request(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 2);
    EXPECT_EQ(2, i);

    observer.set([&](auto&& request) {
        EXPECT_EQ("Hello", request.message().value());
        auto msg = service.make_response_message();
        msg.set_value(123);
        request.respond(msg);
    });

    client.async_request(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_EQ(123, response.message().value());
        ++i;
        return STOP;
    });

    while (i < 3);
    EXPECT_EQ(3, i);

    observer.clear();

    client.async_request(msg, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 4);
    EXPECT_EQ(4, i);
}

TEST_F(ObserversTest, RawServiceTerminal)
{
    auto conn = std::make_unique<LocalConnection>(leafA, leafB);
    while (rawService.get_binding_state() == RELEASED || rawClient.get_subscription_state() == UNSUBSCRIBED);

    MessageObserver<decltype(rawService)> observer(rawService);

    std::atomic<int> i{0};
    auto data = std::vector<char>{69};
    rawClient.async_request(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & DEAF);
        ++i;
        return STOP;
    });

    while (i < 1);
    EXPECT_EQ(1, i);

    observer.start();

    rawClient.async_request(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 2);
    EXPECT_EQ(2, i);

    observer.set([&](auto&& request) {
        EXPECT_EQ(std::vector<char>{69}, request.data());
        auto data = std::vector<char>{123};
        request.respond(data);
    });

    rawClient.async_request(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_EQ(std::vector<char>{123}, response.data());
        ++i;
        return STOP;
    });

    while (i < 3);
    EXPECT_EQ(3, i);

    observer.clear();

    rawClient.async_request(data, [&](auto& res, auto&& response) {
        EXPECT_EQ(Success(), res);
        EXPECT_TRUE(response.flags() & IGNORED);
        ++i;
        return STOP;
    });

    while (i < 4);
    EXPECT_EQ(4, i);
}
