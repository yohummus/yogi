#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"
#include "../../src/config.h"

#include <gmock/gmock.h>

#include <thread>
#include <chrono>


struct TcpLibraryTest : public testing::Test
{
	void* scheduler;
	void* leaf;
	void* node;
    void* leafConn;
    void* nodeConn;

	virtual void SetUp() override
	{
		ASSERT_EQ(CHIRP_OK, CHIRP_Initialise());

		scheduler = helpers::make_scheduler();
		leaf      = helpers::make_leaf(scheduler);
		node      = helpers::make_node(scheduler);

        make_connections();
	}

	virtual void TearDown() override
	{
		ASSERT_EQ(CHIRP_OK, CHIRP_Shutdown());
	}

    void make_connections()
    {
        void* server = helpers::make_tcp_server(scheduler, "Hello");
        void* client = helpers::make_tcp_client(scheduler, "Hello");

        helpers::TcpAcceptHandler acceptFn;
        int res = CHIRP_AsyncTcpAccept(server, -1, helpers::TcpAcceptHandler::fn,
            &acceptFn);
        EXPECT_EQ(CHIRP_OK, res);

        helpers::TcpConnectHandler connectFn;
        res = CHIRP_AsyncTcpConnect(client, "::1", CHIRP_DEFAULT_TCP_PORT, -1,
            helpers::TcpConnectHandler::fn, &connectFn);
        EXPECT_EQ(CHIRP_OK, res);

        acceptFn.wait();
        EXPECT_EQ(CHIRP_OK, acceptFn.lastErrorCode);
        EXPECT_NE(nullptr, acceptFn.lastTcpConnection);
        leafConn = acceptFn.lastTcpConnection;

        connectFn.wait();
        EXPECT_EQ(CHIRP_OK, connectFn.lastErrorCode);
        EXPECT_NE(nullptr, connectFn.lastTcpConnection);
        nodeConn = connectFn.lastTcpConnection;

        helpers::destroy(server);
        helpers::destroy(client);
    }
};

TEST_F(TcpLibraryTest, ClientServerConnection)
{
	int res = CHIRP_AssignConnection(leafConn, leaf, -1);
	EXPECT_EQ(CHIRP_OK, res);

	res = CHIRP_AssignConnection(nodeConn, node, -1);
	EXPECT_EQ(CHIRP_OK, res);
}

TEST_F(TcpLibraryTest, CancelAccept)
{
	void* server = helpers::make_tcp_server(scheduler, "Hello");

	helpers::TcpAcceptHandler acceptFn;
	int res = CHIRP_AsyncTcpAccept(server, -1, helpers::TcpAcceptHandler::fn,
		&acceptFn);
	EXPECT_EQ(CHIRP_OK, res);

	res = CHIRP_CancelTcpAccept(server);
	EXPECT_EQ(CHIRP_OK, res);

	acceptFn.wait();
	EXPECT_EQ(CHIRP_ERR_CANCELED, acceptFn.lastErrorCode);
	EXPECT_EQ(nullptr, acceptFn.lastTcpConnection);
}

TEST_F(TcpLibraryTest, CancelConnect)
{
	void* client = helpers::make_tcp_client(scheduler, "Hello");

	helpers::TcpConnectHandler connectFn;
	int res = CHIRP_AsyncTcpConnect(client, "::1", CHIRP_DEFAULT_TCP_PORT, -1,
		helpers::TcpConnectHandler::fn, &connectFn);
	EXPECT_EQ(CHIRP_OK, res);

	res = CHIRP_CancelTcpConnect(client);
	EXPECT_EQ(CHIRP_OK, res);

	connectFn.wait();
	EXPECT_EQ(CHIRP_ERR_CANCELED, connectFn.lastErrorCode);
	EXPECT_EQ(nullptr, connectFn.lastTcpConnection);
}

TEST_F(TcpLibraryTest, CancelAwaitDeath)
{
    helpers::AwaitDeathHandler awaitDeathFn;
    int res = CHIRP_AsyncAwaitConnectionDeath(leafConn, helpers::AwaitDeathHandler::fn,
        &awaitDeathFn);
    EXPECT_EQ(CHIRP_OK, res);

    res = CHIRP_CancelAwaitConnectionDeath(leafConn);
    EXPECT_EQ(CHIRP_OK, res);

    awaitDeathFn.wait();
    EXPECT_EQ(CHIRP_ERR_CANCELED, awaitDeathFn.lastErrorCode);
}

TEST_F(TcpLibraryTest, ConnectionClosed)
{
    helpers::AwaitDeathHandler awaitDeathFn;
    int res = CHIRP_AsyncAwaitConnectionDeath(leafConn, helpers::AwaitDeathHandler::fn,
        &awaitDeathFn);
    EXPECT_EQ(CHIRP_OK, res);

    res = CHIRP_AssignConnection(leafConn, leaf, -1);
    EXPECT_EQ(CHIRP_OK, res);

    helpers::destroy(nodeConn);
    awaitDeathFn.wait();
    EXPECT_TRUE(awaitDeathFn.lastErrorCode == CHIRP_ERR_CONNECTION_CLOSED
        || awaitDeathFn.lastErrorCode == CHIRP_ERR_SOCKET_BROKEN);
}

TEST_F(TcpLibraryTest, ConnectionTimeout)
{
    helpers::AwaitDeathHandler awaitDeathFn;
    int res = CHIRP_AsyncAwaitConnectionDeath(leafConn, helpers::AwaitDeathHandler::fn,
        &awaitDeathFn);
    EXPECT_EQ(CHIRP_OK, res);

    res = CHIRP_AssignConnection(leafConn, leaf, 5);
    EXPECT_EQ(CHIRP_OK, res);

    awaitDeathFn.wait();
    EXPECT_EQ(CHIRP_ERR_TIMEOUT, awaitDeathFn.lastErrorCode);
}

TEST_F(TcpLibraryTest, ConnectionInformation)
{
    char buffer[100];

    int res = CHIRP_GetConnectionDescription(leafConn, buffer, sizeof(buffer));
    EXPECT_EQ(CHIRP_OK, res);
    buffer[4] = '\0';
    EXPECT_STREQ("::1:", buffer);

    res = CHIRP_GetRemoteVersion(leafConn, buffer, sizeof(buffer));
    EXPECT_EQ(CHIRP_OK, res);
    EXPECT_STREQ(CHIRP_GetVersion(), buffer);

    unsigned n;
    res = CHIRP_GetRemoteIdentification(leafConn, buffer, sizeof(buffer), &n);
    EXPECT_EQ(CHIRP_OK, res);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_EQ(6, n);
}
