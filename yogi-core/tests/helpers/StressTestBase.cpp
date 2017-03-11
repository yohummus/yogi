#include "StressTestBase.hpp"


namespace helpers {

void StressTestBase::SetUp()
{
    ASSERT_EQ(YOGI_OK, YOGI_Initialise());
}

void StressTestBase::TearDown()
{
    int res = YOGI_Shutdown();
    ASSERT_TRUE(res == YOGI_OK || res == YOGI_ERR_NOT_INITIALISED);
}

StressTestBase::task_name_and_fn_pair StressTestBase::take_task()
{
    std::lock_guard<std::mutex> lock{m_mutex};

    if (m_tasks.empty()) {
        return task_name_and_fn_pair{};
    }
    else {
        auto taskPair = m_tasks.back();
        m_tasks.pop_back();
        return taskPair;
    }
}

void StressTestBase::task_thread_fn()
{
    while (true) {
        auto taskPair = take_task();
        if (!taskPair.first) {
            break;
        }

        bool success = taskPair.second();
        if (success) {
            ++m_taskLog[taskPair.first].first;
        }
        else {
            ++m_taskLog[taskPair.first].second;
        }
    }
}

void StressTestBase::clear_stats()
{
    for (auto& entry : m_taskLog) {
        entry.second.first  = 0;
        entry.second.second = 0;
    }
}

void StressTestBase::print_stats()
{
    std::size_t longestName = 0;
    for (auto& entry : m_taskLog) {
        longestName = std::max(longestName, strlen(entry.first));
    }

    std::ios::fmtflags origFmtFlags = std::cout.flags();

    std::cout << std::endl << "Stress test stats:" << std::endl;
    std::cout << std::setw(longestName) << std::left << " Task name"
              << " | Success | Failure |  Total  " << std::endl;
    std::cout << std::setw(longestName) << std::setfill('-') << ""
              << "-+---------+---------+---------" << std::setfill(' ')
              << std::endl;

    for (auto& entry : m_taskLog) {
        const char* name      = entry.first;
        std::size_t successes = entry.second.first;
        std::size_t failures  = entry.second.second;
        std::size_t total     = successes + failures;

        std::cout << std::setw(longestName) << std::left << name << " |"
                  << std::setw(8) << std::right << successes << " |"
                  << std::setw(8) << std::right << failures  << " |"
                  << std::setw(8) << std::right << total     << std::endl;
    }

    std::cout.flags(origFmtFlags);
}

bool StressTestBase::all_tasks_run_successfully()
{
    for (auto& entry : m_taskLog) {
        if (entry.second.first == 0) {
            return false;
        }
    }

    return true;
}

void StressTestBase::run_once(std::size_t numThreads)
{
    auto origTasks = m_tasks;
    shuffle_tasks();

    while (numThreads--) {
        m_taskThreads.push_back(std::thread(&StressTestBase::task_thread_fn,
            this));
    }

    while (!m_taskThreads.empty()) {
        m_taskThreads.back().join();
        m_taskThreads.pop_back();
    }

    print_stats();
    m_tasks = origTasks;
}

void StressTestBase::shuffle_tasks()
{
    std::lock_guard<std::mutex> lock{m_mutex};

    std::mt19937 generator{std::random_device{}()};
    std::shuffle(m_tasks.begin(), m_tasks.end(), generator);
}

void StressTestBase::shutdown()
{
    int res = YOGI_Shutdown();
    ASSERT_TRUE(res == YOGI_OK || res == YOGI_ERR_NOT_INITIALISED) << res;
}

void StressTestBase::increase_number_of_tasks_by_factor(std::size_t factor)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    auto original = m_tasks;
    while (--factor) {
        m_tasks.insert(m_tasks.end(), original.begin(), original.end());
    }
}

void StressTestBase::create_log_file()
{
    YOGI_SetLogFile("log.txt", YOGI_VB_TRACE);
}

void StressTestBase::add_task(const char* name, task_type task, std::size_t n)
{
    assert(name && task);

    std::lock_guard<std::mutex> lock{m_mutex};
    m_taskLog[name].first  = 0;
    m_taskLog[name].second = 0;

    while (n--) {
        m_tasks.push_back(std::make_pair(name, task));
    }
}

void StressTestBase::run(std::size_t manifold, std::size_t numThreads)
{
    increase_number_of_tasks_by_factor(manifold);

    run_once(numThreads);
    while (!all_tasks_run_successfully()) {
        std::cout << std::endl << "Not all tasks run successfully. "
                  << "Repeating test..." << std::endl;
        run_once(numThreads);
    }

    shutdown();
}

void StressTestBase::add_create_connection_task(const char* name,
    connection_component_type& conn, std::size_t n)
{
    add_task(name, [&] {
        std::lock_guard<std::mutex> lock1{conn.mutex};
        if (conn.handle) {
            return false;
        }

        std::lock_guard<std::mutex> lock2{conn.leafNodeA.mutex};
        if (!conn.leafNodeA.handle) {
            return false;
        }

        std::lock_guard<std::mutex> lock3{conn.leafNodeB.mutex};
        if (!conn.leafNodeB.handle) {
            return false;
        }

        conn.handle = helpers::make_connection(conn.leafNodeA.handle,
            conn.leafNodeB.handle);
        return true;
    }, n);
}

void StressTestBase::add_destroy_connection_task(const char* name,
    connection_component_type& conn, std::size_t n)
{
    add_task(name, [&] {
        std::lock_guard<std::mutex> lock{conn.mutex};
        if (conn.handle) {
            helpers::destroy(conn.handle);
            conn.handle = nullptr;
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_create_leaf_task(const char* name,
    leaf_component_type& leaf, std::size_t n)
{
    add_task(name, [&] {
        std::lock(leaf.mutex, leaf.sched.mutex);
        std::lock_guard<std::mutex> lock1{leaf.mutex, std::adopt_lock};
        std::lock_guard<std::mutex> lock2{leaf.sched.mutex,
            std::adopt_lock};

        if (!leaf.handle && leaf.sched.handle) {
            leaf.handle = helpers::make_leaf(leaf.sched.handle);
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_create_node_task(const char* name,
    node_component_type& node, std::size_t n)
{
    add_task(name, [&] {
        std::lock(node.mutex, node.sched.mutex);
        std::lock_guard<std::mutex> lock1{node.mutex, std::adopt_lock};
        std::lock_guard<std::mutex> lock2{node.sched.mutex,
            std::adopt_lock};

        if (!node.handle && node.sched.handle) {
            node.handle = helpers::make_node(node.sched.handle);
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_create_terminal_task(const char* name,
    terminal_component_type& tm, int type, const char* tmName, std::size_t n)
{
    add_task(name, [&, type, tmName] {
        std::lock(tm.mutex, tm.leaf.mutex);
        std::lock_guard<std::mutex> lock1{tm.mutex, std::adopt_lock};
        std::lock_guard<std::mutex> lock2{tm.leaf.mutex, std::adopt_lock};

        if (!tm.handle && tm.leaf.handle) {
            tm.handle = helpers::make_terminal(tm.leaf.handle, type, tmName);
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_create_binding_task(const char* name,
    binding_component_type& bd, const char* targets, std::size_t n)
{
    add_task(name, [&, targets] {
        std::lock(bd.mutex, bd.terminal.mutex);
        std::lock_guard<std::mutex> lock1{bd.mutex, std::adopt_lock};
        std::lock_guard<std::mutex> lock2{bd.terminal.mutex, std::adopt_lock};

        if (!bd.handle && bd.terminal.handle) {
            bd.handle = helpers::make_binding(bd.terminal.handle, targets);
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_destroy_object_task(const char* name,
    component_type& object, std::size_t n)
{
    add_task(name, [&] {
        std::lock_guard<std::mutex> lock{object.mutex};
        if (object.handle && (YOGI_Destroy(object.handle) == YOGI_OK)) {
            object.handle = nullptr;
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_async_await_state_change_task(const char* name,
    binding_component_type& bd, std::size_t n)
{
    add_task(name, [&] {
        std::lock_guard<std::mutex> lock{bd.mutex};
        if (bd.handle) {
            YOGI_AsyncAwaitBindingStateChange(bd.handle,
                &helpers::BindingStateCallbackHandler::fn, &bd.stateHandler);
            return true;
        }

        return false;
    }, n);
}

void StressTestBase::add_async_await_state_change_check_task(const char* name,
    binding_component_type& bd, std::size_t n)
{
    add_task(name, [&] {
        return bd.stateHandler.calls() > 0;
    }, n);
}

#define ADD_CONNECTION_TASKS(conn)                                             \
    add_create_connection_task ("Create  " #conn, scenario.conn);              \
    add_destroy_connection_task("Destroy " #conn, scenario.conn);

#define ADD_LEAF_TASKS(leaf)                                                   \
    add_create_leaf_task   ("Create  " #leaf, scenario.leaf);                  \
    add_destroy_object_task("Destroy " #leaf, scenario.leaf);

#define ADD_NODE_TASKS(node)                                                   \
    add_create_node_task   ("Create  " #node, scenario.node);                  \
    add_destroy_object_task("Destroy " #node, scenario.node);

#define ADD_TM1_TASKS(leaf)                                                    \
    add_create_terminal_task("Create  " #leaf ".tm1", scenario.leaf.tm1,       \
        terminalType, "TM1");                                                  \
    add_destroy_object_task ("Destroy " #leaf ".tm1", scenario.leaf.tm1);

#define ADD_TM2_TASKS(leaf)                                                    \
    add_create_terminal_task("Create  " #leaf ".tm2", scenario.leaf.tm2,       \
        terminalType, "TM2");                                                  \
    add_destroy_object_task ("Destroy " #leaf ".tm2", scenario.leaf.tm2);

#define ADD_BD_TASKS_(bd, target)                                              \
    add_create_binding_task("Create  " #bd, scenario.bd, target);              \
    add_destroy_object_task("Destroy " #bd, scenario.bd);                      \
    add_async_await_state_change_task("Await state change " #bd,               \
        scenario.bd);                                                          \
    add_async_await_state_change_check_task(                                   \
        "Await state change check " #bd, scenario.bd);

#define ADD_BD11_TASKS(leaf) ADD_BD_TASKS_(leaf.bd11, "TM1")
#define ADD_BD12_TASKS(leaf) ADD_BD_TASKS_(leaf.bd12, "TM2")
#define ADD_BD21_TASKS(leaf) ADD_BD_TASKS_(leaf.bd21, "TM1")
#define ADD_BD22_TASKS(leaf) ADD_BD_TASKS_(leaf.bd22, "TM2")


StressTestBase::scenario_2_leafs& StressTestBase::prepare_2_leafs_scenario(
    int terminalType)
{
    auto& scenario = m_scenario2Leafs;

    scenario.leafA.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS    (leafA);
    ADD_TM1_TASKS     (leafA);
    ADD_TM2_TASKS     (leafA);
    ADD_BD11_TASKS    (leafA);
    ADD_BD12_TASKS    (leafA);

    ADD_CONNECTION_TASKS(conn);

    scenario.leafB.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS    (leafB);
    ADD_TM1_TASKS     (leafB);
    ADD_TM2_TASKS     (leafB);
    ADD_BD22_TASKS    (leafB);

    return scenario;
}

StressTestBase::scenario_3_leafs_1_node&
StressTestBase::prepare_3_leafs_1_node_scenario(int terminalType)
{
    auto& scenario = m_scenario3Leafs1Node;

    scenario.node.sched.handle = helpers::make_scheduler(2);
    ADD_NODE_TASKS(node);

    scenario.leafA.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS      (leafA);
    ADD_CONNECTION_TASKS(connA);
    ADD_TM1_TASKS       (leafA);
    ADD_TM2_TASKS       (leafA);
    ADD_BD21_TASKS      (leafA);
    ADD_BD22_TASKS      (leafA);

    scenario.leafB.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS      (leafB);
    ADD_CONNECTION_TASKS(connB);
    ADD_TM1_TASKS       (leafB);
    ADD_BD11_TASKS      (leafB);

    scenario.leafC.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS      (leafC);
    ADD_CONNECTION_TASKS(connC);
    ADD_TM1_TASKS       (leafC);
    ADD_TM2_TASKS       (leafC);

    return scenario;
}

StressTestBase::scenario_3_leafs_3_nodes&
StressTestBase::prepare_3_leafs_3_nodes_scenario(int terminalType)
{
    auto& scenario = m_scenario3Leafs3Nodes;

    scenario.nodeA.sched.handle = helpers::make_scheduler(2);
    ADD_NODE_TASKS(nodeA);
    ADD_CONNECTION_TASKS(connD);
    scenario.nodeB.sched.handle = helpers::make_scheduler(2);
    ADD_NODE_TASKS(nodeB);
    ADD_CONNECTION_TASKS(connE);
    scenario.nodeC.sched.handle = helpers::make_scheduler(2);
    ADD_NODE_TASKS(nodeC);

    scenario.leafA.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS      (leafA);
    ADD_CONNECTION_TASKS(connA);
    ADD_TM1_TASKS       (leafA);
    ADD_TM2_TASKS       (leafA);
    ADD_BD21_TASKS      (leafA);
    ADD_BD22_TASKS      (leafA);

    scenario.leafB.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS      (leafB);
    ADD_CONNECTION_TASKS(connB);
    ADD_TM1_TASKS       (leafB);
    ADD_BD11_TASKS      (leafB);

    scenario.leafC.sched.handle = helpers::make_scheduler(2);
    ADD_LEAF_TASKS      (leafC);
    ADD_CONNECTION_TASKS(connC);
    ADD_TM1_TASKS       (leafC);
    ADD_TM2_TASKS       (leafC);

    return scenario;
}

} // namespace helpers
