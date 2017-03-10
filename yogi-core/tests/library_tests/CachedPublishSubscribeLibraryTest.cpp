#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct CachedPublishSubscribeLibraryTest : public testing::Test
{
    void* node;
    void* leafA;
    void* leafB;
    void* connectionA;
    void* connectionB;
    void* terminalA;
    void* terminalB;
    void* binding;

    char buffer[100] = {0};
    helpers::ReceivePublishedMessageHandler rcvMsgFn;

    virtual void SetUp() override
    {
        ASSERT_EQ(CHIRP_OK, CHIRP_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leafA       = make_leaf(make_scheduler());
        leafB       = make_leaf(make_scheduler());
        connectionA = make_connection(leafA, node);
        connectionB = make_connection(leafB, node);
        terminalA   = make_terminal(leafA, CHIRP_TM_CACHEDPUBLISHSUBSCRIBE, "A");
        terminalB   = make_terminal(leafB, CHIRP_TM_CACHEDPUBLISHSUBSCRIBE, "B");
        binding     = make_binding(terminalA, "B");

        await_binding_state(binding, CHIRP_BD_ESTABLISHED);

        // wait a while longer because when the binding is established, it is
        // not guaranteed that the node has actually already subscribed to
        // terminalB on leafB
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(CHIRP_OK, CHIRP_Shutdown());
    }

    void publish_receive(const char* data, unsigned bufferSize)
    {
        memset(buffer, 0, sizeof(buffer));
        int res = CHIRP_CPS_AsyncReceiveMessage(terminalA, buffer,
            bufferSize, helpers::ReceivePublishedMessageHandler::fn, &rcvMsgFn);
        EXPECT_EQ(CHIRP_OK, res);

        do {
            res = CHIRP_CPS_Publish(terminalB, data, strlen(data) + 1);
        } while (res == CHIRP_ERR_NOT_BOUND);
        EXPECT_EQ(CHIRP_OK, res);

        rcvMsgFn.wait();
    }
};

TEST_F(CachedPublishSubscribeLibraryTest, SuccessfulPublishReceive)
{
    publish_receive("Hello", sizeof(buffer));

    EXPECT_EQ(CHIRP_OK, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_FALSE(rcvMsgFn.cached);
}

TEST_F(CachedPublishSubscribeLibraryTest, BufferTooSmall)
{
    publish_receive("Hello", 1);

    EXPECT_EQ(CHIRP_ERR_BUFFER_TOO_SMALL, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_EQ('H', buffer[0]);
    EXPECT_FALSE(rcvMsgFn.cached);
}

TEST_F(CachedPublishSubscribeLibraryTest, Cache)
{
    publish_receive("Hello", 1);

    // check that there is a cached message
    unsigned bytesWritten;
    int res = CHIRP_CPS_GetCachedMessage(terminalA, buffer, sizeof(buffer),
        &bytesWritten);
    EXPECT_EQ(CHIRP_OK, res);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_EQ(6, bytesWritten);

    // buffer too small
    buffer[0] = 0;
    res = CHIRP_CPS_GetCachedMessage(terminalA, buffer, 1, &bytesWritten);
    EXPECT_EQ(CHIRP_ERR_BUFFER_TOO_SMALL, res);
    EXPECT_EQ('H', buffer[0]);
    EXPECT_EQ(1, bytesWritten);

    // receive cached message
    res = CHIRP_CPS_AsyncReceiveMessage(terminalA, buffer,
        sizeof(buffer), helpers::ReceivePublishedMessageHandler::fn, &rcvMsgFn);
    EXPECT_EQ(CHIRP_OK, res);

    res = CHIRP_Destroy(connectionB);
    EXPECT_EQ(CHIRP_OK, res);
    connectionB = helpers::make_connection(leafB, node);
    rcvMsgFn.wait();

    EXPECT_EQ(CHIRP_OK, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_TRUE(rcvMsgFn.cached);
}
