#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct ProducerConsumerLibraryTest : public testing::Test
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
        ASSERT_EQ(CHIRP_OK, CHIRP_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leafA       = make_leaf(make_scheduler());
        leafB       = make_leaf(make_scheduler());
        connectionA = make_connection(leafA, node);
        connectionB = make_connection(leafB, node);
        terminalA   = make_terminal(leafA, CHIRP_TM_CONSUMER, "T");
        terminalB   = make_terminal(leafB, CHIRP_TM_PRODUCER, "T");

        await_binding_state(terminalA, CHIRP_BD_ESTABLISHED);
        await_subscription_state(terminalB, CHIRP_SB_SUBSCRIBED);

        // wait a while longer because when the binding is established, it is
        // not guaranteed that the node has actually already subscribed itself
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(CHIRP_OK, CHIRP_Shutdown());
    }

    void publish_receive(const char* data, unsigned bufferSize)
    {
        memset(buffer, 0, sizeof(buffer));
        int res = CHIRP_PC_AsyncReceiveMessage(terminalA, buffer,
            bufferSize, helpers::ReceivePublishedMessageHandler::fn, &rcvMsgFn);
        EXPECT_EQ(CHIRP_OK, res);

        do {
            res = CHIRP_PC_Publish(terminalB, data, strlen(data) + 1);
        } while (res == CHIRP_ERR_NOT_BOUND);
        EXPECT_EQ(CHIRP_OK, res);

        rcvMsgFn.wait();
    }
};

TEST_F(ProducerConsumerLibraryTest, SuccessfulOperation)
{
    publish_receive("Hello", sizeof(buffer));

    EXPECT_EQ(CHIRP_OK, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_STREQ("Hello", buffer);
}

TEST_F(ProducerConsumerLibraryTest, BufferTooSmall)
{
    publish_receive("Hello", 1);

    EXPECT_EQ(CHIRP_ERR_BUFFER_TOO_SMALL, rcvMsgFn.lastErrorCode);
    EXPECT_EQ(6, rcvMsgFn.size);
    EXPECT_EQ('H', buffer[0]);
}
