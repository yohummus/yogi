#include <gtest/gtest.h>

#include "../yogi_cpp/api.hpp"
#include "../yogi_cpp/tcp.hpp"
#include "../yogi_cpp/scheduler.hpp"
#include "../yogi_cpp/leaf.hpp"
#include "../yogi_cpp/errors.hpp"
using namespace yogi;
using namespace yogi::errors;

#include <atomic>
using namespace std::chrono;
using namespace std::string_literals;

#define TCP_PORT 12345


struct TcpTest : public testing::Test
{
    Scheduler scheduler;

    std::pair<std::unique_ptr<TcpConnection>, std::unique_ptr<TcpConnection>> make_connection(
        Optional<std::string> ident = none)
    {
        TcpClient client(scheduler, ident);
        TcpServer server(scheduler, "127.0.0.1", TCP_PORT, ident);

        Result acceptResult = Success();
        std::unique_ptr<TcpConnection> acceptConn;
        std::atomic<bool> acceptCalled{false};
        server.async_accept(seconds(5), [&](auto res, auto conn) {
            acceptResult = res;
            acceptConn   = std::move(conn);
            acceptCalled = true;
        });

        Result connectResult = Success();
        std::unique_ptr<TcpConnection> connectConn;
        std::atomic<bool> connectCalled{false};
        client.async_connect("127.0.0.1", TCP_PORT, seconds(5), [&](auto res, auto conn) {
            connectResult = res;
            connectConn   = std::move(conn);
            connectCalled = true;
        });

        while (!acceptCalled || !connectCalled);

        EXPECT_TRUE(!!acceptResult);
        EXPECT_TRUE(!!connectResult);

        return std::make_pair(std::move(acceptConn), std::move(connectConn));
    }
};

TEST_F(TcpTest, Identification)
{
    TcpClient client(scheduler, "Hello"s);
    ASSERT_TRUE(!!client.identification());
    EXPECT_EQ("Hello"s, *client.identification());
}

TEST_F(TcpTest, CancelConnect)
{
    TcpClient client(scheduler);

    Result result = Success();
    std::atomic<bool> called{false};
    client.async_connect("127.0.0.1", TCP_PORT, milliseconds::max(), [&](auto res, auto conn) {
        result = res;
        called = true;
    });

    client.cancel_connect();
    while (!called);
    EXPECT_EQ(Canceled(), result);
}

TEST_F(TcpTest, CancelAccept)
{
    TcpServer server(scheduler, "127.0.0.1", TCP_PORT);

    Result result = Success();
    std::atomic<bool> called{false};
    server.async_accept(milliseconds::max(), [&](auto res, auto conn) {
        result = res;
        called = true;
    });

    server.cancel_accept();
    while (!called);
    EXPECT_EQ(Canceled(), result);
}

TEST_F(TcpTest, ConnectSuccessfully)
{
    make_connection();
}

TEST_F(TcpTest, ConnectionProperties)
{
    auto conns = make_connection("test"s);

    EXPECT_FALSE(conns.first->description().empty());
    EXPECT_FALSE(conns.second->description().empty());

    EXPECT_EQ(get_version(), conns.first->remote_version());
    EXPECT_EQ(get_version(), conns.second->remote_version());

    EXPECT_TRUE(!!conns.first->remote_identification());
    EXPECT_TRUE(!!conns.second->remote_identification());

    EXPECT_EQ("test"s, *conns.first->remote_identification());
    EXPECT_EQ("test"s, *conns.second->remote_identification());
}

TEST_F(TcpTest, AssignConnections)
{
    Leaf leafA(scheduler);
    Leaf leafB(scheduler);

    auto conns = make_connection("test"s);

    EXPECT_NO_THROW(conns.first->assign(leafA, milliseconds::max()));
    EXPECT_NO_THROW(conns.second->assign(leafB, milliseconds::max()));
}

TEST_F(TcpTest, AwaitDeath)
{
    Leaf leafA(scheduler);
    Leaf leafB(scheduler);

    auto conns = make_connection("test"s);

    conns.first->assign(leafA, milliseconds(50));
    conns.second->assign(leafB, milliseconds(50));

    Result result = Success();
    std::atomic<bool> called{false};
    conns.first->async_await_death([&](const Failure& res) {
        result = res;
        called = true;
    });

    conns.second.reset();
    while (!called);
    EXPECT_FALSE(result);
}

TEST_F(TcpTest, CancelAwaitDeath)
{
    Leaf leafA(scheduler);
    Leaf leafB(scheduler);

    auto conns = make_connection("test"s);

    conns.first->assign(leafA, milliseconds(50));
    conns.second->assign(leafB, milliseconds(50));

    Result result = Success();
    std::atomic<bool> called{false};
    conns.first->async_await_death([&](const Failure& res) {
        result = res;
        called = true;
    });

    conns.first->cancel_await_death();
    while (!called);
    EXPECT_EQ(Canceled(), result);
}
