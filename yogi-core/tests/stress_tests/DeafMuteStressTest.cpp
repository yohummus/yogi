#include "../helpers/StressTestBase.hpp"


struct DeafMuteStressTest : helpers::StressTestBase
{
};

TEST_F(DeafMuteStressTest, Scenario2Leafs)
{
    auto& scenario = prepare_2_leafs_scenario(YOGI_TM_DEAFMUTE);
    run();
}

TEST_F(DeafMuteStressTest, Scenario3Leafs1Node)
{
    auto& scenario = prepare_3_leafs_1_node_scenario(YOGI_TM_DEAFMUTE);
    run();
}

TEST_F(DeafMuteStressTest, Scenario3Leafs3Nodes)
{
    auto& scenario = prepare_3_leafs_3_nodes_scenario(YOGI_TM_DEAFMUTE);
    run();
}
