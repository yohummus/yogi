#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"

#include <gmock/gmock.h>


struct CommonLogicLibraryTest : public testing::Test
{
    helpers::BindingStateCallbackHandler stateFn;
    helpers::SubscriptionStateCallbackHandler subscriptionStateFn;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }

    void async_await_state_change(void* binding)
    {
        int res = YOGI_AsyncAwaitBindingStateChange(binding,
            helpers::BindingStateCallbackHandler::fn, &stateFn);
        EXPECT_EQ(YOGI_OK, res);
    }

    void await_state_change(int state)
    {
        stateFn.wait();
        ASSERT_EQ(state, stateFn.newState);
    }

    void run_two_leafs_test(void* leafA, void* leafB)
    {
        // create identical terminals on both leafs
        void* tmAx0 = helpers::make_terminal(leafA, YOGI_TM_DEAFMUTE, "x");
        void* tmBx0 = helpers::make_terminal(leafB, YOGI_TM_DEAFMUTE, "x");

        // create a binding on leaf A for terminals named "x"
        void* bndAx0x = helpers::make_binding(tmAx0, "x");
        helpers::await_binding_state(bndAx0x, YOGI_BD_ESTABLISHED);

        // destroy the terminal on leaf B
        async_await_state_change(bndAx0x);
        helpers::destroy(tmBx0);
        await_state_change(YOGI_BD_RELEASED);

        // re-create the terminal on leaf B
        async_await_state_change(bndAx0x);
        tmBx0 = helpers::make_terminal(leafB, YOGI_TM_DEAFMUTE, "x");
        await_state_change(YOGI_BD_ESTABLISHED);

        // destroy and re-create the binding
        helpers::destroy(bndAx0x);
        bndAx0x = helpers::make_binding(tmAx0, "x");
        helpers::await_binding_state(bndAx0x, YOGI_BD_ESTABLISHED);

        // check number of callback function invocations
        EXPECT_EQ(2, stateFn.calls());
    }
};

TEST_F(CommonLogicLibraryTest, LeafLeafSingleScheduler)
{
    // create a scheduler
    void* scheduler = helpers::make_scheduler(2);

    // create the two leafs
    void* leafA = helpers::make_leaf(scheduler);
    void* leafB = helpers::make_leaf(scheduler);

    // connect the leafs
    helpers::make_connection(leafA, leafB);

    // run the test
    run_two_leafs_test(leafA, leafB);
}

TEST_F(CommonLogicLibraryTest, LeafLeafMultipleSchedulers)
{
    // create the schedulers
    void* schedulerA = helpers::make_scheduler(2);
    void* schedulerB = helpers::make_scheduler(2);

    // create the two leafs
    void* leafA = helpers::make_leaf(schedulerA);
    void* leafB = helpers::make_leaf(schedulerB);

    // connect the leafs
    helpers::make_connection(leafA, leafB);

    // run the test
    run_two_leafs_test(leafA, leafB);
}

TEST_F(CommonLogicLibraryTest, LeafNodeLeaf)
{
    // create the schedulers
    void* schedulerA = helpers::make_scheduler(2);
    void* schedulerB = helpers::make_scheduler(2);

    // create a node and two leafs
    void* node  = helpers::make_node(schedulerA);
    void* leafA = helpers::make_leaf(schedulerA);
    void* leafB = helpers::make_leaf(schedulerB);

    // connect the leafs and nodes
    helpers::make_connection(leafA, node);
    helpers::make_connection(node, leafB);

    // run the test
    run_two_leafs_test(leafA, leafB);
}

TEST_F(CommonLogicLibraryTest, LeafNodeNodeLeaf)
{
    // create the scheduler
    void* scheduler = helpers::make_scheduler(1);

    // create a node and two leafs
    void* nodeA = helpers::make_node(scheduler);
    void* nodeB = helpers::make_node(scheduler);
    void* leafA = helpers::make_leaf(scheduler);
    void* leafB = helpers::make_leaf(scheduler);

    // connect the leafs and nodes
    helpers::make_connection(leafA, nodeA);
    helpers::make_connection(nodeA, nodeB);
    helpers::make_connection(nodeB, leafB);

    // run the test
    run_two_leafs_test(leafA, leafB);
}

TEST_F(CommonLogicLibraryTest, BindingState)
{
    // create a node and two leafs and connect them to the node
    void* scheduler = helpers::make_scheduler();
    void* node      = helpers::make_node(scheduler);
    void* leafA     = helpers::make_leaf(scheduler);
    void* leafB     = helpers::make_leaf(scheduler);

    helpers::make_connection(leafA, node);
    helpers::make_connection(leafB, node);

    // create terminal and binding on leaf A
    void* dummy   = helpers::make_terminal(leafA, YOGI_TM_DEAFMUTE, "x");
    void* binding = helpers::make_binding(dummy, "x");

    // check the binding's state
    int res;
    int state;
    res = YOGI_GetBindingState(binding, &state);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_EQ(YOGI_BD_RELEASED, state);

    // check the binding's state asynchronously
    res = YOGI_AsyncGetBindingState(binding, helpers::BindingStateCallbackHandler::fn,
        &stateFn);
    EXPECT_EQ(YOGI_OK, res);

    stateFn.wait();
    EXPECT_EQ(YOGI_BD_RELEASED, stateFn.newState);
    EXPECT_EQ(YOGI_OK, stateFn.lastErrorCode);

    // create terminal on leaf B and check for state change
    res = YOGI_AsyncAwaitBindingStateChange(binding,
        helpers::BindingStateCallbackHandler::fn, &stateFn);
    EXPECT_EQ(YOGI_OK, res);

    void* terminal = helpers::make_terminal(leafB, YOGI_TM_DEAFMUTE, "x");
    stateFn.wait();
    EXPECT_EQ(YOGI_OK, stateFn.lastErrorCode);
    EXPECT_EQ(YOGI_BD_ESTABLISHED, stateFn.newState);

    // check the binding's state again
    res = YOGI_GetBindingState(binding, &state);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_EQ(YOGI_BD_ESTABLISHED, state);

    // check number of callback function invocations
    EXPECT_EQ(2, stateFn.calls());

    // wait for a change in the binding's state asynchronously and cancel the
    // operation
    res = YOGI_AsyncAwaitBindingStateChange(binding,
        helpers::BindingStateCallbackHandler::fn, &stateFn);
    EXPECT_EQ(YOGI_OK, res);

    res = YOGI_CancelAwaitBindingStateChange(binding);
    EXPECT_EQ(YOGI_OK, res);

    stateFn.wait();
    EXPECT_EQ(YOGI_ERR_CANCELED, stateFn.lastErrorCode);

    // destroy the terminal on leaf B
    helpers::destroy(terminal);

    // check that the callback function does not get called again
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
    EXPECT_EQ(3, stateFn.calls());
}

TEST_F(CommonLogicLibraryTest, SubscriptionState)
{
    // create a node and two leafs and connect them to the node
    void* scheduler = helpers::make_scheduler();
    void* node      = helpers::make_node(scheduler);
    void* leafA     = helpers::make_leaf(scheduler);
    void* leafB     = helpers::make_leaf(scheduler);

    helpers::make_connection(leafA, node);
    helpers::make_connection(leafB, node);

    // create terminals and on both leafs
    void* dummy    = helpers::make_terminal(leafA, YOGI_TM_PUBLISHSUBSCRIBE,
        "x");
    void* terminal = helpers::make_terminal(leafB, YOGI_TM_PUBLISHSUBSCRIBE,
        "x");

    // check the subscription state
    int res;
    int state;
    res = YOGI_GetSubscriptionState(terminal, &state);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_EQ(YOGI_SB_UNSUBSCRIBED, state);

    // check the subscription state asynchronously
    res = YOGI_AsyncGetSubscriptionState(terminal,
        helpers::SubscriptionStateCallbackHandler::fn, &subscriptionStateFn);
    EXPECT_EQ(YOGI_OK, res);

    subscriptionStateFn.wait();
    EXPECT_EQ(YOGI_SB_UNSUBSCRIBED, subscriptionStateFn.newState);
    EXPECT_EQ(YOGI_OK, subscriptionStateFn.lastErrorCode);

    // create a binding and check the subscription state
    res = YOGI_AsyncAwaitSubscriptionStateChange(terminal,
        helpers::SubscriptionStateCallbackHandler::fn, &subscriptionStateFn);
    void* binding = helpers::make_binding(dummy, "x");
    subscriptionStateFn.wait();
    EXPECT_EQ(YOGI_OK, subscriptionStateFn.lastErrorCode);
    EXPECT_EQ(YOGI_SB_SUBSCRIBED, subscriptionStateFn.newState);

    // check the subscription state again
    res = YOGI_GetSubscriptionState(terminal, &state);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_EQ(YOGI_SB_SUBSCRIBED, state);

    // check number of callback function invocations
    EXPECT_EQ(2, subscriptionStateFn.calls());

    // wait for a change in the subscription state asynchronously and cancel the
    // operation
    res = YOGI_AsyncAwaitSubscriptionStateChange(terminal,
        helpers::SubscriptionStateCallbackHandler::fn, &subscriptionStateFn);
    EXPECT_EQ(YOGI_OK, res);

    res = YOGI_CancelAwaitSubscriptionStateChange(terminal);
    EXPECT_EQ(YOGI_OK, res);

    subscriptionStateFn.wait();
    EXPECT_EQ(YOGI_ERR_CANCELED, subscriptionStateFn.lastErrorCode);

    // destroy the terminal
    helpers::destroy(terminal);

    // check that the callback function does not get called again
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
    EXPECT_EQ(3, subscriptionStateFn.calls());
}
