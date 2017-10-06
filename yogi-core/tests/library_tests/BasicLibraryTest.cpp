#include "../helpers/library_helpers.hpp"

#include <gmock/gmock.h>


struct BasicLibraryTest : public testing::Test
{
    void* scheduler;
    void* leaf;
    void* node;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        scheduler = helpers::make_scheduler();
        leaf      = helpers::make_leaf(scheduler);
        node      = helpers::make_node(scheduler);
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }
};

TEST_F(BasicLibraryTest, Scheduler)
{
    // change the worker thread pool size
    int res = YOGI_SetSchedulerThreadPoolSize(scheduler, 0);
    EXPECT_EQ(YOGI_ERR_INVALID_PARAM, res);

    res = YOGI_SetSchedulerThreadPoolSize(scheduler, 99999);
    EXPECT_EQ(YOGI_ERR_INVALID_PARAM, res);

    res = YOGI_SetSchedulerThreadPoolSize(scheduler, 5);
    EXPECT_EQ(YOGI_OK, res);
}

TEST_F(BasicLibraryTest, ObjectDependencies)
{
    // create a connection between the leaf and the node
    void* connection = helpers::make_connection(leaf, node);

    // create a terminal and a binding
    void* terminal = helpers::make_terminal(leaf, YOGI_TM_DEAFMUTE, "A");
    void* binding  = helpers::make_binding(terminal, "B");

    // try to destroy the terminal
    int res = YOGI_Destroy(terminal);
    ASSERT_EQ(YOGI_ERR_OBJECT_STILL_USED, res);

    // destroy the binding
    res = YOGI_Destroy(binding);
    ASSERT_EQ(YOGI_OK, res);

    // try to destroy the leaf
    res = YOGI_Destroy(leaf);
    ASSERT_EQ(YOGI_ERR_OBJECT_STILL_USED, res);

    // destroy the terminal
    res = YOGI_Destroy(terminal);
    ASSERT_EQ(YOGI_OK, res);

    // try to destroy the leaf
    res = YOGI_Destroy(leaf);
    ASSERT_EQ(YOGI_ERR_OBJECT_STILL_USED, res);

    // try to destroy the node
    res = YOGI_Destroy(node);
    ASSERT_EQ(YOGI_ERR_OBJECT_STILL_USED, res);

    // destroy the connection
    res = YOGI_Destroy(connection);
    ASSERT_EQ(YOGI_OK, res);

    // destroy the leaf
    res = YOGI_Destroy(leaf);
    ASSERT_EQ(YOGI_OK, res);

    // try to destroy the scheduler
    res = YOGI_Destroy(scheduler);
    ASSERT_EQ(YOGI_ERR_OBJECT_STILL_USED, res);

    // destroy the node
    res = YOGI_Destroy(node);
    ASSERT_EQ(YOGI_OK, res);

    // destroy the scheduler
    res = YOGI_Destroy(scheduler);
    ASSERT_EQ(YOGI_OK, res);
}

TEST_F(BasicLibraryTest, ConnectTreeElements)
{
    // try to connect the communicators twice
    void* connection = helpers::make_connection(leaf, node);

    int res = YOGI_CreateLocalConnection(&connection, leaf, node);
    ASSERT_EQ(YOGI_ERR_ALREADY_CONNECTED, res);
}

TEST_F(BasicLibraryTest, CreateTerminalsWithSameIdentifier)
{
    void* terminal = helpers::make_terminal(leaf, YOGI_TM_DEAFMUTE, "x");

    // try to create an terminal with the same name and signature
    int res = YOGI_CreateTerminal(&terminal, leaf, YOGI_TM_DEAFMUTE, "x", 0);
    EXPECT_EQ(YOGI_ERR_AMBIGUOUS_IDENTIFIER, res);
}
