#ifndef CHIRP_TESTS_HELPERS_LIBRARY_HELPERS_HPP
#define CHIRP_TESTS_HELPERS_LIBRARY_HELPERS_HPP

#include "../../src/chirp.h"
#include "../../src/config.h"

#include <gmock/gmock.h>

#include <thread>
#include <mutex>
#include <condition_variable>


namespace helpers {
namespace {

void* make_scheduler(int numThreads = 2)
{
    void* scheduler = nullptr;
    int res = CHIRP_CreateScheduler(&scheduler);
    EXPECT_EQ(CHIRP_OK, res);
    res = CHIRP_SetSchedulerThreadPoolSize(scheduler, numThreads);
    EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, scheduler);
    return scheduler;
}

void* make_leaf(void* scheduler)
{
    void* leaf = nullptr;
    int res = CHIRP_CreateLeaf(&leaf, scheduler);
    EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, leaf);
    return leaf;
}

void* make_node(void* scheduler)
{
    void* node = nullptr;
    int res = CHIRP_CreateNode(&node, scheduler);
    EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, node);
    return node;
}

void* make_connection(void* leafNodeA, void* leafNodeB)
{
    void* connection = nullptr;
    int res = CHIRP_CreateLocalConnection(&connection, leafNodeA, leafNodeB);
    EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, connection);
    return connection;
}

void* make_tcp_server(void* scheduler, std::string ident = std::string{})
{
	void* server = nullptr;
	int res = CHIRP_CreateTcpServer(&server, scheduler, "::1",
		CHIRP_DEFAULT_TCP_PORT, ident.c_str(),
        static_cast<unsigned>(ident.size() + 1));
	EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, server);
	return server;
}

void* make_tcp_client(void* scheduler, std::string ident = std::string{})
{
	void* client = nullptr;
	int res = CHIRP_CreateTcpClient(&client, scheduler, ident.c_str(),
        static_cast<unsigned>(ident.size() + 1));
	EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, client);
	return client;
}

void* make_terminal(void* leaf, int type, const char* name, int signature = 0)
{
    void* terminal = nullptr;
    int res = CHIRP_CreateTerminal(&terminal, leaf, type, name, signature);
    EXPECT_EQ(CHIRP_OK, res);
    EXPECT_NE(nullptr, terminal);
    return terminal;
}

void* make_binding(void* terminal, const char* name)
{
    void* binding = nullptr;
    int res = CHIRP_CreateBinding(&binding, terminal, name);
    EXPECT_EQ(CHIRP_OK, res);
	EXPECT_NE(nullptr, binding);
    return binding;
}

void await_binding_state(void* binding, int state)
{
    int curState;
    do {
        int res = CHIRP_GetBindingState(binding, &curState);
        EXPECT_EQ(CHIRP_OK, res);
        std::this_thread::yield();
    } while (curState != state);
}

void await_subscription_state(void* terminal, int state)
{
    int curState;
    do {
        int res = CHIRP_GetSubscriptionState(terminal, &curState);
        EXPECT_EQ(CHIRP_OK, res);
        std::this_thread::yield();
    } while (curState != state);
}

void destroy(void* obj, int tries = 1, std::chrono::microseconds interval = std::chrono::microseconds(10))
{
    int res;

    for (int i = 0; i < tries; ++i) {
        res = CHIRP_Destroy(obj);
        if (res == CHIRP_OK) {
            break;
        }

        std::this_thread::sleep_for(interval);
    }

    ASSERT_EQ(CHIRP_OK, res);
}

std::pair<void*, void*> connect_tcp(void* server, void* leafNodeA, void* client, void* leafNodeB)
{
    struct handler_data_t {
        bool called = false;
        int res;
        void* connection;
        std::mutex mutex;
        std::condition_variable cv;
    };

    auto handler = [](int res, void* connection, void* userArg) {
        auto& data = *static_cast<handler_data_t*>(userArg);
        std::lock_guard<std::mutex> lock{data.mutex};
        data.called = true;
        data.res = res;
        data.connection = connection;
        data.cv.notify_all();
    };

    handler_data_t acceptHandlerData;
    int res = CHIRP_AsyncTcpAccept(server, -1, handler, &acceptHandlerData);
    EXPECT_EQ(CHIRP_OK, res);

    handler_data_t connectHandlerData;
    res = CHIRP_AsyncTcpConnect(client, "::1", CHIRP_DEFAULT_TCP_PORT, -1, handler, &connectHandlerData);
    EXPECT_EQ(CHIRP_OK, res);

    {{
        std::unique_lock<std::mutex> lock{acceptHandlerData.mutex};
        acceptHandlerData.cv.wait(lock, [&] { return acceptHandlerData.called; });
        EXPECT_EQ(acceptHandlerData.res, CHIRP_OK);
        EXPECT_NE(acceptHandlerData.connection, nullptr);
    }}

    {{
        std::unique_lock<std::mutex> lock{connectHandlerData.mutex};
        connectHandlerData.cv.wait(lock, [&] { return connectHandlerData.called; });
        EXPECT_EQ(connectHandlerData.res, CHIRP_OK);
        EXPECT_NE(connectHandlerData.connection, nullptr);
    }}

    res = CHIRP_AssignConnection(acceptHandlerData.connection, leafNodeA, -1);
    EXPECT_EQ(CHIRP_OK, res);

    res = CHIRP_AssignConnection(connectHandlerData.connection, leafNodeB, -1);
    EXPECT_EQ(CHIRP_OK, res);

    return std::make_pair(acceptHandlerData.connection, connectHandlerData.connection);
}

} // anonymous namespace
} // namespace helpers

#endif // CHIRP_TESTS_HELPERS_LIBRARY_HELPERS_HPP
