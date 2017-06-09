#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct NodeLibraryTest : public testing::Test
{
    void* node;
    void* leaf;
    void* connection;

    helpers::ReceivePublishedMessageHandler rcvMsgFn;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leaf       = make_leaf(make_scheduler());
        connection = make_connection(leaf, node);
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }
};

TEST_F(NodeLibraryTest, GetKnownTerminals)
{
    helpers::make_terminal(leaf, YOGI_TM_PUBLISHSUBSCRIBE, "Hello", 123);
    helpers::make_terminal(leaf, YOGI_TM_SCATTERGATHER,    "World", 98342);

    {{
        std::vector<unsigned char> expected{YOGI_TM_PUBLISHSUBSCRIBE, 0x7B,
            0x00, 0x00, 0x00, 'H', 'e', 'l', 'l', 'o', '\0',
            YOGI_TM_SCATTERGATHER, 0x26, 0x80, 0x01, 0x00, 'W', 'o', 'r', 'l',
            'd', '\0'};
        std::vector<unsigned char> buffer(expected.size());

        unsigned numTerminals = 0;
        while (numTerminals != 2) {
            int res = YOGI_GetKnownTerminals(node, buffer.data(),
                static_cast<unsigned>(buffer.size()), &numTerminals);
            EXPECT_EQ(YOGI_OK, res);
        }
        EXPECT_EQ(expected, buffer);
    }}

    {{
        std::vector<unsigned char> expected{YOGI_TM_PUBLISHSUBSCRIBE, 0x7B,
            0x00, 0x00, 0x00, 'H', 'e', 'l', 'l', 'o', '\0'};
        std::vector<unsigned char> buffer(expected.size() + 5);

        unsigned numTerminals = 0;
        int res = YOGI_GetKnownTerminals(node, buffer.data(),
            static_cast<unsigned>(buffer.size()), &numTerminals);
        EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, res);
        EXPECT_EQ(1, numTerminals);

        buffer.resize(expected.size());
        EXPECT_EQ(expected, buffer);
    }}
}


TEST_F(NodeLibraryTest, AsyncAwaitKnownTerminalsChange)
{
    helpers::AwaitKnownTerminalsChangeHandler handlerFn;

    std::vector<unsigned char> expected{0x01, YOGI_TM_PUBLISHSUBSCRIBE, 0x7B,
        0x00, 0x00, 0x00, 'H', 'e', 'l', 'l', 'o', '\0', 0x00};
    std::vector<unsigned char> buffer(expected.size());
    int res = YOGI_AsyncAwaitKnownTerminalsChange(node, buffer.data(),
        static_cast<unsigned>(buffer.size()),
        helpers::AwaitKnownTerminalsChangeHandler::fn, &handlerFn);
    EXPECT_EQ(YOGI_OK, res);

    void* terminal = helpers::make_terminal(leaf, YOGI_TM_PUBLISHSUBSCRIBE,
        "Hello", 123);
    handlerFn.wait();
    EXPECT_EQ(1, handlerFn.calls());
    EXPECT_EQ(YOGI_OK, handlerFn.lastErrorCode);
    EXPECT_EQ(expected, buffer);

    res = YOGI_AsyncAwaitKnownTerminalsChange(node, buffer.data(), 5,
        helpers::AwaitKnownTerminalsChangeHandler::fn, &handlerFn);
    EXPECT_EQ(YOGI_OK, res);

    helpers::destroy(terminal);

    handlerFn.wait();
    EXPECT_EQ(2, handlerFn.calls());
    EXPECT_EQ(YOGI_ERR_BUFFER_TOO_SMALL, handlerFn.lastErrorCode);
}

TEST_F(NodeLibraryTest, CancelAwaitKnownTerminalsChange)
{
    helpers::AwaitKnownTerminalsChangeHandler handlerFn;

    std::vector<unsigned char> buffer(10);
    int res = YOGI_AsyncAwaitKnownTerminalsChange(node, buffer.data(),
        static_cast<unsigned>(buffer.size()),
        helpers::AwaitKnownTerminalsChangeHandler::fn, &handlerFn);
    EXPECT_EQ(YOGI_OK, res);

    res = YOGI_CancelAwaitKnownTerminalsChange(node);
    EXPECT_EQ(YOGI_OK, res);

    handlerFn.wait();
    EXPECT_EQ(1, handlerFn.calls());
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerFn.lastErrorCode);
}

