#include "../../src/connections/local/LocalConnection.hpp"
using namespace yogi::interfaces;
using namespace yogi::connections::local;

#include "../mocks/SchedulerMock.hpp"
#include "../mocks/CommunicatorMock.hpp"
#include "../mocks/MessageMock.hpp"

#include <thread>


struct LocalConnectionTest : public testing::Test
{
    boost::asio::io_service                              ioServiceA;
    boost::asio::io_service                              ioServiceB;
    std::shared_ptr<mocks::SchedulerMock>                schedulerA;
    std::shared_ptr<mocks::SchedulerMock>                schedulerB;
    std::shared_ptr<mocks::CommunicatorMock>             communicatorA;
    std::shared_ptr<mocks::CommunicatorMock>             communicatorB;
    std::shared_ptr<connections::local::LocalConnection> uut;
    interfaces::IConnection*                             connectionA;
    interfaces::IConnection*                             connectionB;

    virtual void SetUp() override
    {
        schedulerA = std::make_shared<mocks::SchedulerMock>(ioServiceA);
        schedulerB = std::make_shared<mocks::SchedulerMock>(ioServiceB);

        communicatorA = std::make_shared<mocks::CommunicatorMock>(*schedulerA);
        communicatorB = std::make_shared<mocks::CommunicatorMock>(*schedulerB);

        // create the UUT and check if on_new_connection() and
        // on_connection_started() get called
        EXPECT_CALL(*communicatorA, on_new_connection(_))
            .WillOnce(Invoke([&](IConnection& connection) {
                connectionA = &connection;
                })
            );
        EXPECT_CALL(*communicatorA, on_connection_started(_));
        EXPECT_CALL(*communicatorB, on_new_connection(_))
            .WillOnce(Invoke([&](IConnection& connection) {
                connectionB = &connection;
                })
            );
        EXPECT_CALL(*communicatorB, on_connection_started(_));

        uut = std::make_shared<LocalConnection>(*communicatorA, *communicatorB);

        EXPECT_NE(connectionA, connectionB);
    }

    virtual void TearDown() override
    {
        EXPECT_CALL(*communicatorA, on_connection_destroyed(Ref(*connectionA)))
            .Times(AnyNumber());
        EXPECT_CALL(*communicatorB, on_connection_destroyed(Ref(*connectionB)))
            .Times(AnyNumber());

        run_io_services_while([&] {
            uut.reset();
        });
    }

    void run_io_services()
    {
        ioServiceA.reset();
        ioServiceB.reset();

        while (ioServiceA.run() + ioServiceB.run()) {
            ioServiceA.reset();
            ioServiceB.reset();
        }
    }

    template <typename TFn>
    void run_io_services_while(TFn fn)
    {
        std::atomic<bool> run{true};

        std::thread th([&] {
            while (run) {
                run_io_services();
                std::this_thread::yield();
            }
        });

        fn();

        run = false;
        th.join();
    }
};

TEST_F(LocalConnectionTest, CreateDestroy)
{
    // parts of this are already tested in SetUp()
    EXPECT_CALL(*communicatorA, on_connection_destroyed(Ref(*connectionA)));
    EXPECT_CALL(*communicatorB, on_connection_destroyed(Ref(*connectionB)));

    run_io_services_while([&] {
        uut.reset();
    });
}

TEST_F(LocalConnectionTest, FirstConnectionRejected)
{
    EXPECT_CALL(*communicatorB, on_new_connection(_));
    EXPECT_CALL(*communicatorB, on_connection_started(_));
    EXPECT_CALL(*communicatorA, on_new_connection(_))
        .WillOnce(Throw(std::exception{}));
    EXPECT_CALL(*communicatorB, on_connection_destroyed(_));
    EXPECT_CALL(*communicatorA, on_connection_destroyed(_))
        .Times(0);

    run_io_services_while([&] {
        EXPECT_THROW({
            std::make_shared<LocalConnection>(*communicatorA, *communicatorB);
        }, std::exception);
    });
}

TEST_F(LocalConnectionTest, SecondConnectionRejected)
{
    EXPECT_CALL(*communicatorB, on_new_connection(_))
        .WillOnce(Throw(std::exception{}));
    EXPECT_CALL(*communicatorA, on_new_connection(_))
        .Times(0);
    EXPECT_CALL(*communicatorA, on_connection_destroyed(_))
        .Times(0);
    EXPECT_CALL(*communicatorB, on_connection_destroyed(_))
        .Times(0);

    run_io_services_while([&] {
        EXPECT_THROW({
            std::make_shared<LocalConnection>(*communicatorA, *communicatorB);
        }, std::exception);
    });
}

TEST_F(LocalConnectionTest, Description)
{
	EXPECT_EQ("Local Connection", connectionA->description());
	EXPECT_EQ("Local Connection", connectionB->description());
}

TEST_F(LocalConnectionTest, RemoteVersion)
{
	EXPECT_EQ(YOGI_VERSION, connectionA->remote_version());
	EXPECT_EQ(YOGI_VERSION, connectionB->remote_version());
}

TEST_F(LocalConnectionTest, RemoteIdentification)
{
	EXPECT_EQ(std::vector<char>{}, connectionA->remote_identification());
	EXPECT_EQ(std::vector<char>{}, connectionB->remote_identification());
}

TEST_F(LocalConnectionTest, RemoteIsNode)
{
    EXPECT_FALSE(connectionA->remote_is_node());
    EXPECT_FALSE(connectionB->remote_is_node());
}

TEST_F(LocalConnectionTest, SendAtoB)
{
    EXPECT_CALL(*communicatorB, on_message_received_(_, Ref(*connectionB)));
    connectionA->send(mocks::MessageMock{"MSG"});
    run_io_services();
}

TEST_F(LocalConnectionTest, SendBtoA)
{
    EXPECT_CALL(*communicatorA, on_message_received_(_, Ref(*connectionA)));
    connectionB->send(mocks::MessageMock{"MSG"});
    run_io_services();
}

TEST_F(LocalConnectionTest, ExceptionDuringMessageProcessing)
{
    EXPECT_CALL(*communicatorA, on_connection_destroyed(Ref(*connectionA)));
    EXPECT_CALL(*communicatorB, on_connection_destroyed(Ref(*connectionB)));

    EXPECT_CALL(*communicatorB, on_message_received_(_, _))
        .WillOnce(Throw(std::exception{}));

    connectionA->send(mocks::MessageMock{""});
    run_io_services();
}

