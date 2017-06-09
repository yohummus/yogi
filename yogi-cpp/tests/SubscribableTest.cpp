#include <gtest/gtest.h>

#include "../yogi/scheduler.hpp"
#include "../yogi/leaf.hpp"
#include "../yogi/terminals.hpp"
#include "../yogi/connection.hpp"
#include "../yogi/errors.hpp"
using namespace yogi;
using namespace yogi::errors;

#include "proto/yogi_0000d007.h"

#include <atomic>


struct SubscribableTest : public testing::Test
{
    Scheduler                        scheduler;
    Leaf                             leafA;
    Leaf                             leafB;
    ProducerTerminal<yogi_0000d007> terminal;
    LocalConnection                  conn;

    SubscribableTest()
    : leafA(scheduler)
    , leafB(scheduler)
    , terminal(leafA, "T")
    , conn(leafA, leafB)
    {
    }
};

TEST_F(SubscribableTest, GetSubscriptionState)
{
    EXPECT_EQ(UNSUBSCRIBED, terminal.get_subscription_state());
    ConsumerTerminal<yogi_0000d007> terminalB(leafB, "T");
    while (terminal.get_subscription_state() == UNSUBSCRIBED);
    EXPECT_EQ(SUBSCRIBED, terminal.get_subscription_state());
}

TEST_F(SubscribableTest, AsyncGetSubscriptionState)
{
    std::atomic<bool> called{false};
    terminal.async_get_subscription_state([&](auto& res, auto state) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(UNSUBSCRIBED, state);
        called = true;
    });
    while (!called);

    ConsumerTerminal<yogi_0000d007> terminalB(leafB, "T");
    while (terminal.get_subscription_state() == UNSUBSCRIBED);

    called = false;
    terminal.async_get_subscription_state([&](auto& res, auto state) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(SUBSCRIBED, state);
        called = true;
    });
    while (!called);
}

TEST_F(SubscribableTest, AsyncAwaitSubscriptionStateChange)
{
    std::atomic<bool> called{false};
    terminal.async_await_subscription_state_change([&](auto& res, auto state) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(SUBSCRIBED, state);
        called = true;
    });

    ConsumerTerminal<yogi_0000d007> terminalB(leafB, "T");
    while (!called);
}

TEST_F(SubscribableTest, CancelAwaitSubscriptionStateChange)
{
    std::atomic<bool> called{false};
    terminal.async_await_subscription_state_change([&](auto& res, auto state) {
        EXPECT_EQ(Canceled(), res);
        called = true;
    });

    terminal.cancel_await_subscription_state_change();
    while (!called);
}
