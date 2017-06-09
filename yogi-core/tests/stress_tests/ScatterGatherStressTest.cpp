#include "../helpers/StressTestBase.hpp"


struct ScatterGatherStressTest : helpers::StressTestBase
{
    void add_request_task(const char* name, terminal_component_type& tm,
        helpers::ReceiveGatheredMessageHandler& handlerFn)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_SG_AsyncScatterGather(tm.handle, "Hello",
                    sizeof("Hello"), nullptr, 0,
                    helpers::ReceiveGatheredMessageHandler::fn, &handlerFn);
                EXPECT_TRUE(res > 0 || res == YOGI_ERR_NOT_BOUND) << res;
                return res > 0;
            }

            return false;
        }, 10);
    }

    void add_cancel_request_task(const char* name, terminal_component_type& tm)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_SG_CancelScatterGather(tm.handle, 1);
                EXPECT_TRUE(res == YOGI_OK || res == YOGI_ERR_INVALID_ID)
                    << res;
                return res == YOGI_OK;
            }

            return false;
        }, 3);
    }

    void add_check_request_handler_task(const char* name,
        helpers::ReceiveGatheredMessageHandler& handlerFn)
    {
        add_task(name, [&] {
            return handlerFn.calls() > 0;
        });
    }

    void add_respond_task(const char* name, terminal_component_type& tm,
        helpers::ReceiveScatteredMessageHandler& handlerFn)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_SG_AsyncReceiveScatteredMessage(tm.handle,
                    nullptr, 0, helpers::ReceiveScatteredMessageHandler::fn,
                    &handlerFn);

                EXPECT_TRUE(res == YOGI_OK || res == YOGI_ERR_NOT_BOUND
                    || res == YOGI_ERR_ASYNC_OPERATION_RUNNING) << res;
                return res == YOGI_OK;
            }

            return false;
        }, 5);
    }

    void add_cancel_respond_task(const char* name, terminal_component_type& tm)
    {
        add_task(name, [&] {
            std::lock_guard<std::mutex> lock{tm.mutex};
            if (tm.handle) {
                int res = YOGI_SG_CancelReceiveScatteredMessage(tm.handle);
                EXPECT_TRUE(res == YOGI_OK);
                return true;
            }

            return false;
        });
    }

    void add_check_respond_handler_task(const char* name,
        helpers::ReceiveScatteredMessageHandler& handlerFn)
    {
        add_task(name, [&] {
            return handlerFn.calls() > 0;
        });
    }
};

#define ADD_REQUEST_TASK(tm, handlerFn)                                        \
    add_request_task("Request " #tm, scenario.tm, handlerFn);                  \
    add_check_request_handler_task("Check handler " #handlerFn, handlerFn);    \
    add_cancel_request_task("Cancel request " #tm, scenario.tm);

#define ADD_RESPOND_TASK(tm, handlerFn)                                        \
    add_respond_task("Respond " #tm, scenario.tm, handlerFn);                  \
    add_check_respond_handler_task("Check handler " #handlerFn, handlerFn);    \
    add_cancel_respond_task("Cancel respond " #tm, scenario.tm);

TEST_F(ScatterGatherStressTest, Scenario2Leafs)
{
    auto& scenario = prepare_2_leafs_scenario(YOGI_TM_SCATTERGATHER);

    helpers::ReceiveScatteredMessageHandler tm1AScatterHandlerFn;
    ADD_RESPOND_TASK(leafA.tm1, tm1AScatterHandlerFn);
    helpers::ReceiveGatheredMessageHandler tm2AGatherHandlerFn;
    ADD_REQUEST_TASK(leafA.tm2, tm2AGatherHandlerFn);

    helpers::ReceiveScatteredMessageHandler tm2BScatterHandlerFn;
    ADD_RESPOND_TASK(leafB.tm2, tm2BScatterHandlerFn);
    helpers::ReceiveGatheredMessageHandler tm1BGatherHandlerFn;
    ADD_REQUEST_TASK(leafB.tm1, tm1BGatherHandlerFn);
    helpers::ReceiveGatheredMessageHandler tm2BGatherHandlerFn;
    ADD_REQUEST_TASK(leafB.tm2, tm2BGatherHandlerFn);

    run();
}

TEST_F(ScatterGatherStressTest, Scenario3Leafs1Node)
{
    auto& scenario = prepare_3_leafs_1_node_scenario(YOGI_TM_SCATTERGATHER);

    helpers::ReceiveGatheredMessageHandler tm1AGatherHandlerFn;
    ADD_REQUEST_TASK(leafA.tm1, tm1AGatherHandlerFn);
    helpers::ReceiveScatteredMessageHandler tm2AScatterHandlerFn;
    ADD_RESPOND_TASK(leafA.tm2, tm2AScatterHandlerFn);

    helpers::ReceiveGatheredMessageHandler tm1BGatherHandlerFn;
    ADD_REQUEST_TASK(leafB.tm1, tm1BGatherHandlerFn);
    helpers::ReceiveScatteredMessageHandler tm1BScatterHandlerFn;
    ADD_RESPOND_TASK(leafB.tm1, tm1BScatterHandlerFn);

    helpers::ReceiveGatheredMessageHandler tm1CGatherHandlerFn;
    ADD_REQUEST_TASK(leafC.tm1, tm1CGatherHandlerFn);
    helpers::ReceiveGatheredMessageHandler tm2CGatherHandlerFn;
    ADD_REQUEST_TASK(leafC.tm2, tm2CGatherHandlerFn);

    run();
}

TEST_F(ScatterGatherStressTest, Scenario3Leafs3Nodes)
{
    auto& scenario = prepare_3_leafs_3_nodes_scenario(YOGI_TM_SCATTERGATHER);

    helpers::ReceiveGatheredMessageHandler tm1AGatherHandlerFn;
    ADD_REQUEST_TASK(leafA.tm1, tm1AGatherHandlerFn);
    helpers::ReceiveScatteredMessageHandler tm2AScatterHandlerFn;
    ADD_RESPOND_TASK(leafA.tm2, tm2AScatterHandlerFn);

    helpers::ReceiveGatheredMessageHandler tm1BGatherHandlerFn;
    ADD_REQUEST_TASK(leafB.tm1, tm1BGatherHandlerFn);
    helpers::ReceiveScatteredMessageHandler tm1BScatterHandlerFn;
    ADD_RESPOND_TASK(leafB.tm1, tm1BScatterHandlerFn);

    helpers::ReceiveGatheredMessageHandler tm1CGatherHandlerFn;
    ADD_REQUEST_TASK(leafC.tm1, tm1CGatherHandlerFn);
    helpers::ReceiveGatheredMessageHandler tm2CGatherHandlerFn;
    ADD_REQUEST_TASK(leafC.tm2, tm2CGatherHandlerFn);

    run();
}
