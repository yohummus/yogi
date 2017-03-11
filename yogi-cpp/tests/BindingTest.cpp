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


struct BindingTest : public testing::Test
{
    Scheduler                        scheduler;
    Leaf                             leafA;
    Leaf                             leafB;
    DeafMuteTerminal<yogi_0000d007> terminal;
    Binding                          binding;
    LocalConnection                  conn;

    BindingTest()
    : leafA(scheduler)
    , leafB(scheduler)
    , terminal(leafA, "T")
    , binding(terminal, "T")
    , conn(leafA, leafB)
    {
    }
};

TEST_F(BindingTest, GetBindingState)
{
    EXPECT_EQ(RELEASED, binding.get_binding_state());
    decltype(terminal) terminalB(leafB, "T");
    while (binding.get_binding_state() == RELEASED);
    EXPECT_EQ(ESTABLISHED, binding.get_binding_state());
}

TEST_F(BindingTest, AsyncGetBindingState)
{
    std::atomic<bool> called{false};
    binding.async_get_binding_state([&](auto& res, auto state) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(RELEASED, state);
        called = true;
    });
    while (!called);

    decltype(terminal) terminalB(leafB, "T");
    while (binding.get_binding_state() == RELEASED);

    called = false;
    binding.async_get_binding_state([&](auto& res, auto state) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(ESTABLISHED, state);
        called = true;
    });
    while (!called);
}

TEST_F(BindingTest, AsyncAwaitBindingStateChange)
{
    std::atomic<bool> called{false};
    binding.async_await_binding_state_change([&](auto& res, auto state) {
        EXPECT_EQ(res, Success());
        EXPECT_EQ(ESTABLISHED, state);
        called = true;
    });

    decltype(terminal) terminalB(leafB, "T");
    while (!called);
}

TEST_F(BindingTest, CancelAwaitBindingStateChange)
{
    std::atomic<bool> called{false};
    binding.async_await_binding_state_change([&](auto& res, auto state) {
        EXPECT_EQ(Canceled(), res);
        called = true;
    });

    binding.cancel_await_binding_state_change();
    while (!called);
}
