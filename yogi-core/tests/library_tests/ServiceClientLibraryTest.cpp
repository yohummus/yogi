#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>

#include <thread>
#include <chrono>


struct ServiceClientLibraryTest : public testing::Test
{
    void* node;
    void* leafA;
    void* leafB;
    void* leafC;
    void* connectionA;
    void* connectionB;
    void* connectionC;
    void* terminalA;
    void* terminalB;
    void* terminalC;

    helpers::ReceiveScatteredMessageHandler rcvScatteredMsgFn;
    helpers::ReceiveGatheredMessageHandler  rcvGatheredMsgFn;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        using namespace helpers;
        node        = make_node(make_scheduler());
        leafA       = make_leaf(make_scheduler());
        leafB       = make_leaf(make_scheduler());
        leafC       = make_leaf(make_scheduler());
        connectionA = make_connection(leafA, node);
        connectionB = make_connection(leafB, node);
        connectionC = make_connection(leafC, node);
        terminalA   = make_terminal(leafA, YOGI_TM_CLIENT, "T");
        terminalB   = make_terminal(leafB, YOGI_TM_SERVICE, "T");
        terminalC   = make_terminal(leafC, YOGI_TM_SERVICE, "T");

        wait_until_system_stable();
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }

    void wait_until_system_stable()
    {
        while (true) {
            helpers::ReceiveGatheredMessageHandler handlerFn;
            int res = YOGI_SC_AsyncRequest(terminalA, nullptr, 0, nullptr,
                0, helpers::ReceiveGatheredMessageHandler::fn, &handlerFn);
            ASSERT_TRUE(res == 1 || res == YOGI_ERR_NOT_BOUND);

            if (res == 1) {
                handlerFn.wait();
                if (!(handlerFn.flags & YOGI_SG_FINISHED)) {
                    handlerFn.wait();
                    ASSERT_TRUE(handlerFn.flags & YOGI_SG_FINISHED);
                    return;
                }
            }

            std::this_thread::sleep_for(std::chrono::microseconds{100});
        }
    }
};

TEST_F(ServiceClientLibraryTest, SuccessfulScatterGatherOperation)
{
    char gathBuffer[10] = {0};
    int res = YOGI_SC_AsyncReceiveRequest(terminalC, gathBuffer,
        sizeof(gathBuffer), helpers::ReceiveScatteredMessageHandler::fn,
        &rcvScatteredMsgFn);
    ASSERT_EQ(YOGI_OK, res);

    char scatBuffer[10] = {0};
    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, scatBuffer,
        sizeof(scatBuffer),
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvScatteredMsgFn.wait();
    EXPECT_GT(rcvScatteredMsgFn.operationId, 0);
    EXPECT_EQ(YOGI_OK, rcvScatteredMsgFn.lastErrorCode);
    EXPECT_EQ(3, rcvScatteredMsgFn.size);
    EXPECT_STREQ("Hi", gathBuffer);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);

    res = YOGI_SC_RespondToRequest(terminalC,
        rcvScatteredMsgFn.operationId, "X", 2);
    EXPECT_EQ(YOGI_OK, res);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_FINISHED, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(2, rcvGatheredMsgFn.size);
    EXPECT_STREQ("X", scatBuffer);
}

TEST_F(ServiceClientLibraryTest, IgnoreScatteredMessage)
{
    char gathBuffer[10] = {0};
    int res = YOGI_SC_AsyncReceiveRequest(terminalC, gathBuffer,
        sizeof(gathBuffer), helpers::ReceiveScatteredMessageHandler::fn,
        &rcvScatteredMsgFn);
    ASSERT_EQ(YOGI_OK, res);

    char scatBuffer[10] = {0};
    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, scatBuffer,
        sizeof(scatBuffer),
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvScatteredMsgFn.wait();
    EXPECT_GT(rcvScatteredMsgFn.operationId, 0);
    EXPECT_EQ(YOGI_OK, rcvScatteredMsgFn.lastErrorCode);
    EXPECT_EQ(3, rcvScatteredMsgFn.size);
    EXPECT_STREQ("Hi", gathBuffer);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);

    res = YOGI_SC_IgnoreRequest(terminalC, rcvScatteredMsgFn.operationId);
    EXPECT_EQ(YOGI_OK, res);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_FINISHED | YOGI_SG_IGNORED, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);
}

TEST_F(ServiceClientLibraryTest, DeafRemoteTerminals)
{
    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, nullptr, 0,
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvGatheredMsgFn.wait(2);
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF | YOGI_SG_FINISHED,
        rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);
}

TEST_F(ServiceClientLibraryTest, AbortAfterFirstGatheredMessage)
{
    rcvGatheredMsgFn.returnValue = 1;
    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, nullptr, 0,
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);
}

TEST_F(ServiceClientLibraryTest, CancelScatterGatherOperation)
{
    int res = YOGI_SC_AsyncReceiveRequest(terminalB, nullptr, 0,
        helpers::ReceiveScatteredMessageHandler::fn, &rcvScatteredMsgFn);
    ASSERT_EQ(YOGI_OK, res);

    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, nullptr, 0,
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvScatteredMsgFn.wait();

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);

    res = YOGI_SC_CancelRequest(terminalA, id);
    ASSERT_EQ(YOGI_OK, res);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_NOFLAGS, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_ERR_CANCELED, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);
}

TEST_F(ServiceClientLibraryTest, CancelReceiveScatteredMessageOperation)
{
    char gathBuffer[10] = {0};
    int res = YOGI_SC_AsyncReceiveRequest(terminalC, gathBuffer,
        sizeof(gathBuffer), helpers::ReceiveScatteredMessageHandler::fn,
        &rcvScatteredMsgFn);
    ASSERT_EQ(YOGI_OK, res);

    res = YOGI_SC_CancelReceiveRequest(terminalC);
    ASSERT_EQ(YOGI_OK, res);

    rcvScatteredMsgFn.wait();
    EXPECT_EQ(YOGI_ERR_CANCELED, rcvScatteredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvScatteredMsgFn.size);
}

TEST_F(ServiceClientLibraryTest, DestroyBindingDuringScatterGatherOperation)
{
    int res = YOGI_SC_AsyncReceiveRequest(terminalB, nullptr, 0,
        helpers::ReceiveScatteredMessageHandler::fn, &rcvScatteredMsgFn);
    ASSERT_EQ(YOGI_OK, res);

    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, nullptr, 0,
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvScatteredMsgFn.wait();

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);

    res = YOGI_Destroy(terminalB);
    ASSERT_EQ(YOGI_OK, res);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_BINDINGDESTROYED | YOGI_SG_FINISHED,
        rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);
}

TEST_F(ServiceClientLibraryTest, DestroyConnectionDuringScatterGatherOperation)
{
    int res = YOGI_SC_AsyncReceiveRequest(terminalB, nullptr, 0,
        helpers::ReceiveScatteredMessageHandler::fn, &rcvScatteredMsgFn);
    ASSERT_EQ(YOGI_OK, res);

    int id = YOGI_SC_AsyncRequest(terminalA, "Hi", 3, nullptr, 0,
        helpers::ReceiveGatheredMessageHandler::fn, &rcvGatheredMsgFn);
    ASSERT_GT(id, 0);

    rcvScatteredMsgFn.wait();

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_DEAF, rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);

    res = YOGI_Destroy(connectionB);
    ASSERT_EQ(YOGI_OK, res);

    rcvGatheredMsgFn.wait();
    EXPECT_EQ(id, rcvGatheredMsgFn.operationId);
    EXPECT_EQ(YOGI_SG_CONNECTIONLOST | YOGI_SG_FINISHED,
        rcvGatheredMsgFn.flags);
    EXPECT_EQ(YOGI_OK, rcvGatheredMsgFn.lastErrorCode);
    EXPECT_EQ(0, rcvGatheredMsgFn.size);
}
