#include "../../src/connections/tcp/TcpConnection.hpp"
#include "../../src/messaging/messages/ScatterGather.hpp"
#include "../../src/messaging/messages/ServiceClient.hpp"
using namespace yogi::base;
using namespace yogi::interfaces;
using namespace yogi::messaging;
using namespace yogi::connections::tcp;

#include "../helpers/Scheduler.hpp"
#include "../mocks/CommunicatorMock.hpp"
#include "../mocks/MessageMock.hpp"

#include <gmock/gmock.h>

#include <thread>


struct TcpConnectionTest : public testing::Test
{
    scheduler_ptr scheduler = std::make_shared<helpers::Scheduler>();
    std::shared_ptr<mocks::LeafCommunicatorMock> leaf
        = std::make_shared<mocks::LeafCommunicatorMock>(*scheduler);
    std::shared_ptr<mocks::NodeCommunicatorMock> node
        = std::make_shared<mocks::NodeCommunicatorMock>(*scheduler);

    tcp_connection_ptr leafConn;
    tcp_connection_ptr nodeConn;

    virtual void SetUp() override
    {
        // connect leafConn and nodeConn via IPv6
        boost::asio::ip::tcp::acceptor acceptor{scheduler->io_service(),
            boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v6(),
            YOGI_DEFAULT_TCP_PORT}};

        boost::asio::ip::tcp::socket socketA{scheduler->io_service()};
        socketA.connect(boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string("::1"),
            YOGI_DEFAULT_TCP_PORT));

        boost::asio::ip::tcp::socket socketB{scheduler->io_service()};
        acceptor.accept(socketB);

        leafConn = std::make_shared<TcpConnection>(*scheduler,
            std::move(socketA), "1.2.3", std::vector<char>{11});
        nodeConn = std::make_shared<TcpConnection>(*scheduler,
            std::move(socketB), "4.5.6", std::vector<char>{22});
    }

    virtual void TearDown()
    {
        EXPECT_CALL(*leaf, on_connection_destroyed(Ref(*leafConn)))
            .Times(AnyNumber());
        EXPECT_CALL(*node, on_connection_destroyed(Ref(*nodeConn)))
            .Times(AnyNumber());
    }

    void await_connection_ready()
    {
        while (true) {
            try {
                leafConn->remote_is_node();
                nodeConn->remote_is_node();
                break;
            }
            catch (const api::ExceptionT<YOGI_ERR_NOT_READY>&) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }

    void prepare_and_await_connection_ready()
    {
        EXPECT_CALL(*leaf, on_new_connection(Ref(*leafConn)));
        EXPECT_CALL(*leaf, on_connection_started(Ref(*leafConn)));
        leafConn->assign(*leaf, std::chrono::milliseconds::max());

        EXPECT_CALL(*node, on_new_connection(Ref(*nodeConn)));
        EXPECT_CALL(*node, on_connection_started(Ref(*nodeConn)));
        nodeConn->assign(*node, std::chrono::milliseconds::max());

        await_connection_ready();
    }

    void wait_for_error(std::atomic<int>* errorCode)
    {
        while (*errorCode == YOGI_OK) {
            std::this_thread::sleep_for(std::chrono::microseconds{100});
        }
    }
};

TEST_F(TcpConnectionTest, Getters)
{
    EXPECT_EQ("1.2.3", leafConn->remote_version());
    EXPECT_EQ(std::vector<char>{11}, leafConn->remote_identification());
    EXPECT_FALSE(leafConn->description().empty());

    EXPECT_EQ("4.5.6", nodeConn->remote_version());
    EXPECT_EQ(std::vector<char>{22}, nodeConn->remote_identification());
    EXPECT_FALSE(nodeConn->description().empty());
}

TEST_F(TcpConnectionTest, Assign)
{
    EXPECT_CALL(*leaf, on_new_connection(Ref(*leafConn)));
    EXPECT_CALL(*leaf, on_connection_started(Ref(*leafConn)));
    leafConn->assign(*leaf, std::chrono::milliseconds::max());

    EXPECT_CALL(*node, on_new_connection(Ref(*nodeConn)));
    EXPECT_CALL(*node, on_connection_started(Ref(*nodeConn)));
    nodeConn->assign(*node, std::chrono::milliseconds::max());

    EXPECT_THROW(leafConn->assign(*leaf, std::chrono::milliseconds::max()),
        api::ExceptionT<YOGI_ERR_ALREADY_ASSIGNED>);

    await_connection_ready();
    EXPECT_EQ(true,  leafConn->remote_is_node());
    EXPECT_EQ(false, nodeConn->remote_is_node());
}

TEST_F(TcpConnectionTest, ExchangeMessages)
{
    prepare_and_await_connection_ready();

    std::vector<char> data{'a', 'b', 'c', 'd'};
    auto msg1 = messages::ServiceClient::Scatter::create(Id{3}, Id{5555},
        Buffer{data.data(), data.size()});
    auto msg2 = messages::ScatterGather::Subscribe::create(Id{879});

    std::atomic<int> msgsRemaining{4};

    EXPECT_CALL(*leaf, on_message_received_(Msg(msg1), Ref(*leafConn)))
        .WillOnce(InvokeWithoutArgs([&]{ --msgsRemaining; }));
    EXPECT_CALL(*leaf, on_message_received_(Msg(msg2), Ref(*leafConn)))
        .WillOnce(InvokeWithoutArgs([&]{ --msgsRemaining; }));
    EXPECT_CALL(*node, on_message_received_(Msg(msg2), Ref(*nodeConn)))
        .WillOnce(InvokeWithoutArgs([&]{ --msgsRemaining; }));
    EXPECT_CALL(*node, on_message_received_(Msg(msg1), Ref(*nodeConn)))
        .WillOnce(InvokeWithoutArgs([&]{ --msgsRemaining; }));

    nodeConn->send(msg1);
    nodeConn->send(msg2);
    leafConn->send(msg2);
    leafConn->send(msg1);

    while (msgsRemaining) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

TEST_F(TcpConnectionTest, AsyncAwaitDeath)
{
    prepare_and_await_connection_ready();

    std::atomic<int> errorCode{YOGI_OK};
    nodeConn->async_await_death([&](const api::Exception& e) {
        errorCode = e.error_code();
    });

    EXPECT_CALL(*leaf, on_connection_destroyed(Ref(*leafConn)));
    leafConn.reset();
    wait_for_error(&errorCode);
    EXPECT_EQ(YOGI_ERR_CONNECTION_CLOSED, errorCode);
}

TEST_F(TcpConnectionTest, CancelAsyncAwaitDeath)
{
    prepare_and_await_connection_ready();

    std::atomic<int> errorCode{YOGI_OK};
    nodeConn->async_await_death([&](const api::Exception& e) {
        errorCode = e.error_code();
    });

    nodeConn->cancel_await_death();
    wait_for_error(&errorCode);
    EXPECT_EQ(YOGI_ERR_CANCELED, errorCode);
}

TEST_F(TcpConnectionTest, PreStartTimeout)
{
    std::atomic<int> errorCode{YOGI_OK};
    leafConn->async_await_death([&](const api::Exception& e) {
        errorCode = e.error_code();
    });

    EXPECT_CALL(*leaf, on_new_connection(Ref(*leafConn)));
    leafConn->assign(*leaf, std::chrono::milliseconds{5});

    wait_for_error(&errorCode);
    EXPECT_EQ(YOGI_ERR_TIMEOUT, errorCode);
}

TEST_F(TcpConnectionTest, HeartbeatTimeout)
{
    EXPECT_CALL(*leaf, on_new_connection(Ref(*leafConn)));
    EXPECT_CALL(*leaf, on_connection_started(Ref(*leafConn)))
        .Times(AnyNumber());
    EXPECT_CALL(*node, on_new_connection(Ref(*nodeConn)));
    EXPECT_CALL(*node, on_connection_started(Ref(*nodeConn)))
        .Times(AnyNumber());

    std::atomic<int> leafErrorCode{YOGI_OK};
    leafConn->async_await_death([&](const api::Exception& e) {
        leafErrorCode = e.error_code();
    });

    std::atomic<int> nodeErrorCode{YOGI_OK};
    nodeConn->async_await_death([&](const api::Exception& e) {
        nodeErrorCode = e.error_code();
    });

    leafConn->assign(*leaf, std::chrono::milliseconds::max());
    nodeConn->assign(*node, std::chrono::milliseconds{10});

    wait_for_error(&leafErrorCode);
    EXPECT_EQ(YOGI_ERR_CONNECTION_CLOSED, leafErrorCode);

    wait_for_error(&nodeErrorCode);
    EXPECT_EQ(YOGI_ERR_TIMEOUT, nodeErrorCode);
}
