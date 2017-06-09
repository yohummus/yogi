#ifndef YOGI_TESTS_HELPERS_STRESSTESTBASE_HPP
#define YOGI_TESTS_HELPERS_STRESSTESTBASE_HPP

#include "../../src/yogi_core.h"
#include "library_helpers.hpp"
#include "CallbackHandler.hpp"

#include <gmock/gmock.h>

#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <random>
#include <atomic>
#include <map>
#include <cassert>


namespace helpers {

class StressTestBase : public testing::Test
{
public:
    typedef std::function<bool ()> task_type;
    typedef std::pair<const char*, task_type> task_name_and_fn_pair;
    typedef std::map<const char*, std::pair<std::atomic<std::size_t>,
        std::atomic<std::size_t>>> task_log_map;

    struct leaf_component_type;

    struct component_type
    {
        std::mutex mutex;
        void*      handle = nullptr;
    };

    struct scheduler_component_type : public component_type
    {
    };

    struct terminal_component_type : public component_type
    {
        leaf_component_type& leaf;

        terminal_component_type(leaf_component_type& leaf_)
            : leaf{leaf_}
        {
        }
    };

    struct binding_component_type : public component_type
    {
        terminal_component_type&             terminal;
        helpers::BindingStateCallbackHandler stateHandler;

        binding_component_type(terminal_component_type& terminal_)
            : terminal{terminal_}
        {
        }
    };

    struct leaf_node_component_type : public component_type
    {
        component_type sched;
    };

    struct leaf_component_type : public leaf_node_component_type
    {
        terminal_component_type tm1;
        terminal_component_type tm2;
        binding_component_type  bd11;
        binding_component_type  bd12;
        binding_component_type  bd21;
        binding_component_type  bd22;

        leaf_component_type()
            : tm1(*this), tm2(*this), bd11(tm1), bd12(tm1), bd21(tm2), bd22(tm2)
        {
        }
    };

    struct node_component_type : public leaf_node_component_type
    {
    };

    struct connection_component_type : public component_type
    {
        leaf_node_component_type& leafNodeA;
        leaf_node_component_type& leafNodeB;

        connection_component_type(leaf_node_component_type& leafNodeA_,
            leaf_node_component_type& leafNodeB_)
            : leafNodeA(leafNodeA_), leafNodeB(leafNodeB_)
        {
        }
    };

    //     +------------------------------------+
    //     | bd12                               |
    // +-------+  bd11              +-------+   |
    // |  tm1  |------------------->|  tm1  |   |
    // +-------+                    +-------+   |
    //     |                            |       |
    // +=======+        conn        +=======+   |
    // | leafA |====================| leafB |   |
    // +=======+                    +=======+   |
    //     |                            |       |
    // +-------+              bd22  +-------+   |
    // |  tm2  |<-------------------|  tm2  |<--+
    // +-------+                    +-------+
    //
    struct scenario_2_leafs
    {
        leaf_component_type       leafA;
        leaf_component_type       leafB;
        connection_component_type conn{leafA, leafB};
    };

    //                   bd11  +-----+  bd11
    // +-----------------------| tm1 |<-----------------------+
    // |                       +-----+ ---------+             |
    // |                          |             |             |
    // |                          |             |             |
    // |                      +========+        |             |
    // |   +-----+            | leafB  |        |   +-----+   |
    // +-->| tm1 |            +========+        +-->| tm1 |<--+
    //     +-----+                ||                +-----+   |
    //        |                   || connB             |      |
    //        |                   ||                   |      |
    //    +=======+    connA   +======+    connC   +=======+  |
    //    | leafA |============| node |============| leafC |  |
    //    +=======+            +======+            +=======+  |
    //        |                                        |      |
    //        |                                        |      |
    //     +-----+  bd22                            +-----+   |
    //     | tm2 |--------------------------------->| tm2 |   |
    //     +-----+                                  +-----+   |
    //        |  bd21                                         |
    //        +-----------------------------------------------+
    //
    struct scenario_3_leafs_1_node
    {
        leaf_component_type       leafA;
        leaf_component_type       leafB;
        leaf_component_type       leafC;
        node_component_type       node;
        connection_component_type connA{leafA, node};
        connection_component_type connB{leafB, node};
        connection_component_type connC{leafC, node};
    };

    //                          bd11  +-----+  bd11
    // +------------------------------| tm1 |<-------------------------------+
    // |                              +-----+------------------+             |
    // |                                 |                     |             |
    // |                                 |                     |             |
    // |                             +========+                |             |
    // |   +-----+                   | leafB  |                |   +-----+   |
    // +-->| tm1 |                   +========+                +-->| tm1 |<--+
    //     +-----+                       ||                        +-----+   |
    //        |                          || connB                     |      |
    //        |    conn          conn    ||    conn          conn     |      |
    //    +=======+  A  +=======+  D  +=======+  E  +=======+  C  +=======+  |
    //    | leafA |=====| nodeA |=====| nodeB |=====| nodeC |=====| leafC |  |
    //    +=======+     +=======+     +=======+     +=======+     +=======+  |
    //        |                                                       |      |
    //        |                                                       |      |
    //     +-----+  bd22                                           +-----+   |
    //     | tm2 |------------------------------------------------>| tm2 |   |
    //     +-----+                                                 +-----+   |
    //        |  bd21                                                        |
    //        +--------------------------------------------------------------+
    //
    struct scenario_3_leafs_3_nodes
    {
        leaf_component_type       leafA;
        leaf_component_type       leafB;
        leaf_component_type       leafC;
        node_component_type       nodeA;
        node_component_type       nodeB;
        node_component_type       nodeC;
        connection_component_type connA{leafA, nodeA};
        connection_component_type connB{leafB, nodeB};
        connection_component_type connC{leafC, nodeC};
        connection_component_type connD{nodeA, nodeB};
        connection_component_type connE{nodeB, nodeC};
    };

private:
    std::mutex                         m_mutex;
    std::vector<task_name_and_fn_pair> m_tasks;
    std::vector<std::thread>           m_taskThreads;
    task_log_map                       m_taskLog;
    scenario_2_leafs                   m_scenario2Leafs;
    scenario_3_leafs_1_node            m_scenario3Leafs1Node;
    scenario_3_leafs_3_nodes           m_scenario3Leafs3Nodes;

private:
    virtual void SetUp() override final;
    virtual void TearDown() override final;
    task_name_and_fn_pair take_task();
    void task_thread_fn();
    void clear_stats();
    void print_stats();
    bool all_tasks_run_successfully();
    void run_once(std::size_t numThreads);
    void increase_number_of_tasks_by_factor(std::size_t factor);
    void shuffle_tasks();
    void shutdown();

protected:
    void create_log_file();

    void add_task(const char* name, task_type task, std::size_t n = 1);
    void run(std::size_t manifold = 10000, std::size_t numThreads = 6);

    void add_create_connection_task(const char* name,
        connection_component_type& conn, std::size_t n = 1);
    void add_destroy_connection_task(const char* name,
        connection_component_type& conn, std::size_t n = 1);
    void add_create_leaf_task(const char* name, leaf_component_type& leaf,
        std::size_t n = 1);
    void add_create_node_task(const char* name, node_component_type& node,
        std::size_t n = 1);
    void add_create_terminal_task(const char* name, terminal_component_type& tm,
        int type, const char* tmName, std::size_t n = 1);
    void add_create_binding_task(const char* name, binding_component_type& bd,
        const char* targets, std::size_t n = 1);
    void add_destroy_object_task(const char* name, component_type& object,
        std::size_t n = 1);
    void add_async_await_state_change_task(const char* name,
        binding_component_type& bd, std::size_t n = 1);
    void add_async_await_state_change_check_task(const char* name,
        binding_component_type& bd, std::size_t n = 1);

    scenario_2_leafs& prepare_2_leafs_scenario(int terminalType);
    scenario_3_leafs_1_node& prepare_3_leafs_1_node_scenario(int terminalType);
    scenario_3_leafs_3_nodes& prepare_3_leafs_3_nodes_scenario(
        int terminalType);
};

} // namespace helpers

#endif // YOGI_TESTS_HELPERS_STRESSTESTBASE_HPP
