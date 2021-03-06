#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct CachedProducerConsumerLibraryTest : public testing::Test
{
    void* node;
    void* leafA;
    void* leafB;
    void* connectionA;
    void* connectionB;
    void* terminalA;
    void* terminalB;

    char buffer[100] = {0};
    helpers::ReceivePublishedMessageHandler rcvMsgFn;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leafA       = make_leaf(make_scheduler());
        leafB       = make_leaf(make_scheduler());
        connectionA = make_connection(leafA, node);
        connectionB = make_connection(leafB, node);
        terminalA   = make_terminal(leafA, YOGI_TM_CACHEDCONSUMER, "T");
        terminalB   = make_terminal(leafB, YOGI_TM_CACHEDPRODUCER, "T");

        await_binding_state(terminalA, YOGI_BD_ESTABLISHED);
        await_subscription_state(terminalB, YOGI_SB_SUBSCRIBED);

        // wait a while longer because when the binding is established, it is
        // not guaranteed that the node has actually already subscribed itself
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }

    void publish_receive(const char* data, unsigned bufferSize)
    {
        memset(buffer, 0, sizeof(buffer));
        int res = YOGI_CPC_AsyncReceiveMessage(terminalA, buffer,
            bufferSize, helpers::ReceivePublishedMessageHandler::fn, &rcvMsgFn);
        EXPECT_EQ(YOGI_OK, res);

        do {
            res = YOGI_CPC_Publish(terminalB, data, strlen(data) + 1);
        } while (res == YOGI_ERR_NOT_BOUND);
        EXPECT_EQ(YOGI_OK, res);

        rcvMsgFn.wait();
    }
};

TEST_F(CachedProducerConsumerLibraryTest, SuccessfulPublishReceive)
{
    publish_receive("Hello", sizeof(buffer));

    EXPECT_EQ(YOGI_OK, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_FALSE(rcvMsgFn.cached);
}

TEST_F(CachedProducerConsumerLibraryTest, BufferTooSmall)
{
    publish_receive("Hello", 1);

    EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_EQ('H', buffer[0]);
    EXPECT_FALSE(rcvMsgFn.cached);
}

TEST_F(CachedProducerConsumerLibraryTest, Cache)
{
    publish_receive("Hello", 1);

    // check that there is a cached message
    unsigned bytesWritten;
    int res = YOGI_CPC_GetCachedMessage(terminalA, buffer, sizeof(buffer),
        &bytesWritten);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_EQ(6, bytesWritten);

    // buffer too small
    buffer[0] = 0;
    res = YOGI_CPC_GetCachedMessage(terminalA, buffer, 1, &bytesWritten);
    EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, res);
    EXPECT_EQ('H', buffer[0]);
    EXPECT_EQ(1, bytesWritten);

    // receive cached message
    res = YOGI_CPC_AsyncReceiveMessage(terminalA, buffer,
        sizeof(buffer), helpers::ReceivePublishedMessageHandler::fn, &rcvMsgFn);
    EXPECT_EQ(YOGI_OK, res);

    res = YOGI_Destroy(connectionB);
    EXPECT_EQ(YOGI_OK, res);
    connectionB = helpers::make_connection(leafB, node);
    rcvMsgFn.wait();

    EXPECT_EQ(YOGI_OK, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_STREQ("Hello", buffer);
    EXPECT_TRUE(rcvMsgFn.cached);
}
