#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct CachedMasterSlaveLibraryTest : public testing::Test
{
    void* node;
    void* leafA;
    void* leafB;
    void* connectionA;
    void* connectionB;
    void* terminalA;
    void* terminalB;

    char masterBuffer[100] = {0};
    char slaveBuffer[100] = {0};
    helpers::ReceivePublishedMessageHandler masterRcvMsgFn;
    helpers::ReceivePublishedMessageHandler slaveRcvMsgFn;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leafA       = make_leaf(make_scheduler());
        leafB       = make_leaf(make_scheduler());
        connectionA = make_connection(leafA, node);
        connectionB = make_connection(leafB, node);
        terminalA   = make_terminal(leafA, YOGI_TM_CACHEDSLAVE, "T");
        terminalB   = make_terminal(leafB, YOGI_TM_CACHEDMASTER, "T");

        await_binding_state(terminalA, YOGI_BD_ESTABLISHED);
        await_binding_state(terminalB, YOGI_BD_ESTABLISHED);
        await_subscription_state(terminalA, YOGI_SB_SUBSCRIBED);
        await_subscription_state(terminalB, YOGI_SB_SUBSCRIBED);

        // wait a while longer because when the binding is established, it is
        // not guaranteed that the node has actually already subscribed itself
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }

    void publish(void* terminal, const char* data)
    {
        int res;
        do {
            res = YOGI_CMS_Publish(terminal, data, strlen(data) + 1);
        } while (res == YOGI_ERR_NOT_BOUND);
        EXPECT_EQ(YOGI_OK, res);
    }

    void async_receive_master(unsigned bufferSize)
    {
        memset(masterBuffer, 0, sizeof(masterBuffer));
        int res = YOGI_CMS_AsyncReceiveMessage(terminalB, masterBuffer,
            bufferSize, helpers::ReceivePublishedMessageHandler::fn,
            &masterRcvMsgFn);
        EXPECT_EQ(YOGI_OK, res);
    }

    void async_receive_slave(unsigned bufferSize)
    {
        memset(slaveBuffer, 0, sizeof(slaveBuffer));
        int res = YOGI_CMS_AsyncReceiveMessage(terminalA, slaveBuffer,
            bufferSize, helpers::ReceivePublishedMessageHandler::fn,
            &slaveRcvMsgFn);
        EXPECT_EQ(YOGI_OK, res);
    }
};

TEST_F(CachedMasterSlaveLibraryTest, SendFromMaster)
{
    async_receive_slave(sizeof(slaveBuffer));
    publish(terminalB, "Hello");
    slaveRcvMsgFn.wait();

    EXPECT_EQ(YOGI_OK, slaveRcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, slaveRcvMsgFn.size);
    EXPECT_STREQ("Hello", slaveBuffer);
    EXPECT_FALSE(slaveRcvMsgFn.cached);
}

TEST_F(CachedMasterSlaveLibraryTest, SendFromSlave)
{
    async_receive_master(sizeof(masterBuffer));
    publish(terminalA, "Hello");
    masterRcvMsgFn.wait();

    EXPECT_EQ(YOGI_OK, masterRcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, masterRcvMsgFn.size);
    EXPECT_STREQ("Hello", masterBuffer);
    EXPECT_FALSE(masterRcvMsgFn.cached);
}

TEST_F(CachedMasterSlaveLibraryTest, BufferTooSmall)
{
    async_receive_slave(1);
    publish(terminalB, "Hello");
    slaveRcvMsgFn.wait();

    EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, slaveRcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, slaveRcvMsgFn.size);
    EXPECT_EQ('H', slaveBuffer[0]);
    EXPECT_FALSE(slaveRcvMsgFn.cached);
}

TEST_F(CachedMasterSlaveLibraryTest, Cache)
{
    async_receive_slave(sizeof(slaveBuffer));
    publish(terminalB, "Hello");
    slaveRcvMsgFn.wait();

    // check that there is a cached message
    unsigned bytesWritten;
    int res = YOGI_CMS_GetCachedMessage(terminalA, slaveBuffer,
        sizeof(slaveBuffer), &bytesWritten);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_STREQ("Hello", slaveBuffer);
    EXPECT_EQ(6, bytesWritten);

    // buffer too small
    slaveBuffer[0] = 0;
    res = YOGI_CMS_GetCachedMessage(terminalA, slaveBuffer, 1, &bytesWritten);
    EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, res);
    EXPECT_EQ('H', slaveBuffer[0]);
    EXPECT_EQ(1, bytesWritten);

    // receive cached message
    async_receive_slave(sizeof(slaveBuffer));

    res = YOGI_Destroy(connectionB);
    EXPECT_EQ(YOGI_OK, res);
    connectionB = helpers::make_connection(leafB, node);
    slaveRcvMsgFn.wait();

    EXPECT_EQ(YOGI_OK, slaveRcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, slaveRcvMsgFn.size);
    EXPECT_STREQ("Hello", slaveBuffer);
    EXPECT_TRUE(slaveRcvMsgFn.cached);
}
