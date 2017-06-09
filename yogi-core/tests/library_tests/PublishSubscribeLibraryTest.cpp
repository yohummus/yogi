#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct PublishSubscribeLibraryTest : public testing::Test
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
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leafA       = make_leaf(make_scheduler());
        leafB       = make_leaf(make_scheduler());
        connectionA = make_connection(leafA, node);
        connectionB = make_connection(leafB, node);
        terminalA   = make_terminal(leafA, YOGI_TM_PUBLISHSUBSCRIBE, "A");
        terminalB   = make_terminal(leafB, YOGI_TM_PUBLISHSUBSCRIBE, "B");
        binding     = make_binding(terminalA, "B");

        await_binding_state(binding, YOGI_BD_ESTABLISHED);
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }

    void publish_receive(const char* data, unsigned bufferSize)
    {
        memset(buffer, 0, sizeof(buffer));
        int res = YOGI_PS_AsyncReceiveMessage(terminalA, buffer,
            bufferSize, helpers::ReceivePublishedMessageHandler::fn, &rcvMsgFn);
        EXPECT_EQ(YOGI_OK, res);

        do {
            res = YOGI_PS_Publish(terminalB, data, strlen(data) + 1);
        } while (res == YOGI_ERR_NOT_BOUND);
        EXPECT_EQ(YOGI_OK, res);

        rcvMsgFn.wait();
    }
};

TEST_F(PublishSubscribeLibraryTest, SuccessfulOperation)
{
    publish_receive("Hello", sizeof(buffer));

    EXPECT_EQ(YOGI_OK, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_STREQ("Hello", buffer);
}

TEST_F(PublishSubscribeLibraryTest, BufferTooSmall)
{
    publish_receive("Hello", 1);

    EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_EQ('H', buffer[0]);
}
