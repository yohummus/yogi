#include "../helpers/StressTestBase.hpp"


struct CachedPublishSubscribeStressTest : helpers::StressTestBase
{
    void add_publish_task(const char* name, terminal_component_type& tm)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_CPS_Publish(tm.handle, "Hello", sizeof("Hello"));
                EXPECT_TRUE(res == YOGI_OK || res == YOGI_ERR_NOT_BOUND);
                return res == YOGI_OK;
            }

            return false;
        }, 10);
    }

    void add_async_receive_task(const char* name, terminal_component_type& tm,
        helpers::ReceivePublishedMessageHandler& handlerFn)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_CPS_AsyncReceiveMessage(tm.handle,
                    nullptr, 0, helpers::ReceivePublishedMessageHandler::fn,
                    &handlerFn);
                EXPECT_TRUE(res == YOGI_OK
                    || res == YOGI_ERR_ASYNC_OPERATION_RUNNING);
                return res == YOGI_OK;
            }

            return false;
        }, 5);
    }

    void add_cancel_receive_task(const char* name, terminal_component_type& tm)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_CPS_CancelReceiveMessage(tm.handle);
                EXPECT_EQ(YOGI_OK, res);
                return res == YOGI_OK;
            }

            return false;
        });
    }

    void add_check_handler_task(const char* name,
        helpers::ReceivePublishedMessageHandler& handlerFn)
    {
        add_task(name, [&] {
            return handlerFn.calls() > 0;
        });
    }
};

#define ADD_PUBLISH_TASKS(tm)                                                  \
    add_publish_task("Publish " #tm, scenario.tm);

#define ADD_RECEIVE_TASKS(tm, handlerFn)                                       \
    add_async_receive_task("Receive " #tm, scenario.tm, handlerFn);            \
    add_check_handler_task("Check handler " #handlerFn, handlerFn);            \
    add_cancel_receive_task("Cancel receive " #tm, scenario.tm);

TEST_F(CachedPublishSubscribeStressTest, Scenario2Leafs)
{
    auto& scenario = prepare_2_leafs_scenario(YOGI_TM_CACHEDPUBLISHSUBSCRIBE);

    helpers::ReceivePublishedMessageHandler tm1AHandlerFn;
    ADD_RECEIVE_TASKS(leafA.tm1, tm1AHandlerFn);
    ADD_PUBLISH_TASKS(leafA.tm2);

    helpers::ReceivePublishedMessageHandler tm2BHandlerFn;
    ADD_RECEIVE_TASKS(leafB.tm2, tm2BHandlerFn);
    ADD_PUBLISH_TASKS(leafB.tm1);
    ADD_PUBLISH_TASKS(leafB.tm2);

    run();
}

TEST_F(CachedPublishSubscribeStressTest, Scenario3Leafs1Node)
{
    auto& scenario = prepare_3_leafs_1_node_scenario(
        YOGI_TM_CACHEDPUBLISHSUBSCRIBE);

    helpers::ReceivePublishedMessageHandler tm2AHandlerFn;
    ADD_RECEIVE_TASKS(leafA.tm2, tm2AHandlerFn);
    ADD_PUBLISH_TASKS(leafA.tm1);

    helpers::ReceivePublishedMessageHandler tm1BHandlerFn;
    ADD_RECEIVE_TASKS(leafB.tm1, tm1BHandlerFn);
    ADD_PUBLISH_TASKS(leafB.tm1);

    ADD_PUBLISH_TASKS(leafC.tm1);
    ADD_PUBLISH_TASKS(leafC.tm2);

    run();
}

TEST_F(CachedPublishSubscribeStressTest, Scenario3Leafs3Nodes)
{
    auto& scenario = prepare_3_leafs_3_nodes_scenario(
        YOGI_TM_CACHEDPUBLISHSUBSCRIBE);

    helpers::ReceivePublishedMessageHandler tm2AHandlerFn;
    ADD_RECEIVE_TASKS(leafA.tm2, tm2AHandlerFn);
    ADD_PUBLISH_TASKS(leafA.tm1);

    helpers::ReceivePublishedMessageHandler tm1BHandlerFn;
    ADD_RECEIVE_TASKS(leafB.tm1, tm1BHandlerFn);
    ADD_PUBLISH_TASKS(leafB.tm1);

    ADD_PUBLISH_TASKS(leafC.tm1);
    ADD_PUBLISH_TASKS(leafC.tm2);

    run();
}
