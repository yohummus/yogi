#include <gtest/gtest.h>

#include "../yogi_cpp/tcp.hpp"
#include "../yogi_cpp/scheduler.hpp"
#include "../yogi_cpp/leaf.hpp"
using namespace yogi;

#include <atomic>
#include <thread>
using namespace std::chrono;
using namespace std::string_literals;

#define TCP_PORT 12345u


struct AutoConnectingTcpClientTest : public testing::Test
{
    Scheduler               scheduler;
    Leaf                    leafA;
    Leaf                    leafB;
    AutoConnectingTcpClient client;

    AutoConnectingTcpClientTest()
    : leafA(scheduler)
    , leafB(scheduler)
    , client(leafA, "127.0.0.1", TCP_PORT, seconds(5), "Hello"s)
    {
    }
};

TEST_F(AutoConnectingTcpClientTest, Properties)
{
    EXPECT_EQ(&leafA, &client.endpoint());
    EXPECT_EQ("127.0.0.1", client.host());
    EXPECT_EQ(TCP_PORT, client.port());
    EXPECT_EQ(seconds(5), client.timeout());
    ASSERT_TRUE(!!client.identification());
    EXPECT_EQ("Hello"s, *client.identification());
}

TEST_F(AutoConnectingTcpClientTest, Reconnect)
{
    std::atomic<int> calls{0};
    Result result = Success();
    client.set_connect_observer([&](auto& res, auto& conn) {
        result = res;
        EXPECT_TRUE((res == Success()) ^ !conn);
        ++calls;
    });

    std::atomic<bool> disconnectCalled{false};
    client.set_disconnect_observer([&](auto& res) {
        disconnectCalled = true;
    });

    client.start();

    while (calls < 1) {
        std::this_thread::yield();
    }
    EXPECT_FALSE(result);

    TcpServer server(scheduler, "127.0.0.1", TCP_PORT, "test"s);
    std::unique_ptr<TcpConnection> connection;
    std::atomic<bool> accepted{false};
    server.async_accept(seconds(5), [&](auto& res, auto conn) {
        EXPECT_EQ(res, Success());
        conn->assign(this->leafB, seconds(5));
        connection = std::move(conn);
        accepted = true;
    });

    while (calls < 2 || !accepted) {
        std::this_thread::yield();
    }
    EXPECT_EQ(result, Success());

    connection.reset();

    while (calls < 3) {
        std::this_thread::yield();
    }
    EXPECT_NE(result, Success());

    EXPECT_TRUE(disconnectCalled);
}
