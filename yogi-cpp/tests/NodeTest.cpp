#include <gtest/gtest.h>

#include "../yogi_cpp/connection.hpp"
#include "../yogi_cpp/leaf.hpp"
#include "../yogi_cpp/node.hpp"
#include "../yogi_cpp/terminals.hpp"
#include "../yogi_cpp/errors.hpp"
using namespace yogi;
using namespace yogi::errors;

#include <atomic>


struct NodeTest : public testing::Test
{
    Scheduler       scheduler;
    Leaf            leaf;
    Node            node;
    LocalConnection connection;

    NodeTest()
    : leaf(scheduler)
    , node(scheduler)
    , connection(leaf, node)
    {
    }
};

TEST_F(NodeTest, GetKnownTerminals)
{
    RawProducerTerminal a(leaf, "One", Signature(123));
    RawServiceTerminal  b(leaf, "Two", Signature(456));

    while (node.get_known_terminals().size() < 2);
    EXPECT_EQ(2u, node.get_known_terminals().size());

    auto terminals = node.get_known_terminals();
    EXPECT_EQ(PRODUCER,       terminals[0].type);
    EXPECT_EQ(Signature(123), terminals[0].signature);
    EXPECT_EQ("One",          terminals[0].name);
    EXPECT_EQ(SERVICE,        terminals[1].type);
    EXPECT_EQ(Signature(456), terminals[1].signature);
    EXPECT_EQ("Two",          terminals[1].name);
}

TEST_F(NodeTest, AwaitKnownTerminalsChange)
{
    std::atomic<bool> called{false};
    node.async_await_known_terminals_change([&](auto& result, auto&& info, auto change) {
        EXPECT_EQ(Success(),      result);
        EXPECT_EQ(PRODUCER,       info.type);
        EXPECT_EQ(Signature(123), info.signature);
        EXPECT_EQ("One",          info.name);
        EXPECT_EQ(ADDED,          change);
        called = true;
    });

    auto terminal = std::make_unique<RawProducerTerminal>(leaf, "One", Signature(123));
    while (!called);

    called = false;
    node.async_await_known_terminals_change([&](auto& result, auto&& info, auto change) {
        EXPECT_EQ(Success(),      result);
        EXPECT_EQ(PRODUCER,       info.type);
        EXPECT_EQ(Signature(123), info.signature);
        EXPECT_EQ("One",          info.name);
        EXPECT_EQ(REMOVED,        change);
        called = true;
    });

    terminal.reset();
    while (!called);
}

TEST_F(NodeTest, CancelAwaitKnownTerminalsChange)
{
    std::atomic<bool> called{false};
    node.async_await_known_terminals_change([&](auto& result, auto&& info, auto change) {
        EXPECT_EQ(Canceled(), result);
        called = true;
    });

    node.cancel_await_known_terminals_change();
    while (!called);
}
