#include "../helpers/StressTestBase.hpp"

#include <queue>
#include <cstring>


struct TcpStressTest : helpers::StressTestBase
{
    scheduler_component_type   serverScheduler;
    component_type             server;
	leaf_component_type        serverLeaf;
    terminal_component_type    serverTerminal;
    helpers::TcpAcceptHandler  acceptHandlerFn;
    helpers::AwaitDeathHandler serverAwaitDeathFn;
	std::mutex                 serverConnectionsMutex;
	std::queue<void*>          serverConnections;

    scheduler_component_type   clientScheduler;
    component_type             client;
	leaf_component_type	       clientLeaf;
    terminal_component_type    clientTerminal;
    helpers::TcpConnectHandler connectHandlerFn;
    helpers::AwaitDeathHandler clientAwaitDeathFn;
	std::mutex                 clientConnectionsMutex;
	std::queue<void*>          clientConnections;

	TcpStressTest()
        : serverTerminal  (serverLeaf)
		, acceptHandlerFn ([&](void* conn) { push_server_connection(conn); })
        , clientTerminal  (clientLeaf)
		, connectHandlerFn([&](void* conn) { push_client_connection(conn); })
	{
	}

	void* pop_connection(std::queue<void*>& queue, std::mutex& mutex)
	{
		std::lock_guard<std::mutex> lock{mutex};
		if (queue.empty()) {
			return nullptr;
		}
		else {
			void* connection = queue.front();
			queue.pop();
			return connection;
		}

	}
	void* pop_server_connection()
	{
		return pop_connection(serverConnections, serverConnectionsMutex);
	}

	void* pop_client_connection()
	{
		return pop_connection(clientConnections, clientConnectionsMutex);
	}

	void push_server_connection(void* connection)
	{
		std::lock_guard<std::mutex> lock{serverConnectionsMutex};
		serverConnections.push(connection);
	}

	void push_client_connection(void* connection)
	{
		std::lock_guard<std::mutex> lock{clientConnectionsMutex};
		clientConnections.push(connection);
	}

    void add_create_server_task()
    {
        add_task("Create server", [&] {
            std::lock_guard<std::mutex> lock{server.mutex};
            if (server.handle) {
                return false;
            }

            server.handle = helpers::make_tcp_server(serverScheduler.handle,
                "Hello");
            return true;
        }, 30);
    }

    void add_accept_task()
    {
        add_task("Accept", [&] {
            std::lock_guard<std::mutex> lock{server.mutex};
            if (server.handle) {
                int res = YOGI_AsyncTcpAccept(server.handle, 3,
                    helpers::TcpAcceptHandler::fn, &acceptHandlerFn);
                EXPECT_TRUE(res == YOGI_OK
                    || res == YOGI_ERR_ASYNC_OPERATION_RUNNING) << res;
                return res == YOGI_OK;
            }

            return false;
        }, 30);
    }

    void add_cancel_accept_task()
    {
        add_task("Cancel accept", [&] {
            std::lock_guard<std::mutex> lock{server.mutex};
            if (server.handle) {
                int res = YOGI_CancelTcpAccept(server.handle);
                EXPECT_TRUE(res == YOGI_OK) << res;
                return res == YOGI_OK;
            }

            return false;
        }, 5);
    }

    void add_check_accept_handler_task()
    {
		add_task("Check accept handler", [&] {
			return acceptHandlerFn.calls() > 0;
		});
    }

	void add_assign_server_connection_task()
	{
		add_task("Assign server connection", [&] {
			void* connection = pop_server_connection();

			std::lock_guard<std::mutex> lock{serverLeaf.mutex};
			int res = YOGI_AssignConnection(connection, serverLeaf.handle, 3);

			push_server_connection(connection);
			return res == YOGI_OK;
		}, 10);
	}

    void add_server_connection_await_death_task()
    {
        add_task("Await server connection death", [&] {
            void* connection = pop_server_connection();

            int res = YOGI_AsyncAwaitConnectionDeath(connection,
                helpers::AwaitDeathHandler::fn, &serverAwaitDeathFn);

            push_server_connection(connection);
            return res == YOGI_OK;
        }, 10);
    }

    void add_server_connection_cancel_await_death_task()
    {
        add_task("Cancel await server connection death", [&] {
            void* connection = pop_server_connection();

            int res = YOGI_CancelAwaitConnectionDeath(connection);

            push_server_connection(connection);
            return res == YOGI_OK;
        }, 3);
    }

    void add_destroy_server_connection_task()
    {
        add_task("Destroy server connection", [&] {
            void* connection = pop_server_connection();

            int res = YOGI_Destroy(connection);
            if (res != YOGI_OK) {
                push_server_connection(connection);
            }

            return res == YOGI_OK;
        }, 30);
    }

    void add_create_client_task()
    {
        add_task("Create client", [&] {
            std::lock_guard<std::mutex> lock{client.mutex};
            if (client.handle) {
                return false;
            }

            client.handle = helpers::make_tcp_client(clientScheduler.handle,
                "Hello");
            return true;
        }, 20);
    }

    void add_connect_task()
    {
        add_task("Connect", [&] {
            std::lock_guard<std::mutex> lock{client.mutex};
            if (client.handle) {
                int res = YOGI_AsyncTcpConnect(client.handle, "::1",
                    YOGI_DEFAULT_TCP_PORT, 3, helpers::TcpConnectHandler::fn,
                    &connectHandlerFn);
                EXPECT_TRUE(res == YOGI_OK
                    || res == YOGI_ERR_ASYNC_OPERATION_RUNNING) << res;
                return res == YOGI_OK;
            }

            return false;
        }, 20);
    }

    void add_cancel_connect_task()
    {
        add_task("Cancel connect", [&] {
            std::lock_guard<std::mutex> lock{client.mutex};
            if (client.handle) {
                int res = YOGI_CancelTcpConnect(client.handle);
                EXPECT_TRUE(res == YOGI_OK) << res;
                return res == YOGI_OK;
            }

            return false;
        }, 5);
    }

    void add_check_connect_handler_task()
    {
		add_task("Check connect handler", [&] {
			return connectHandlerFn.calls() > 0;
		});
    }

	void add_assign_client_connection_task()
	{
		add_task("Assign client connection", [&] {
			void* connection = pop_client_connection();

			std::lock_guard<std::mutex> lock{clientLeaf.mutex};
			int res = YOGI_AssignConnection(connection, clientLeaf.handle, 3);

			push_client_connection(connection);
			return res == YOGI_OK;
		}, 10);
	}

    void add_client_connection_await_death_task()
    {
        add_task("Await client connection death", [&] {
            void* connection = pop_client_connection();

            int res = YOGI_AsyncAwaitConnectionDeath(connection,
                helpers::AwaitDeathHandler::fn, &clientAwaitDeathFn);

            push_client_connection(connection);
            return res == YOGI_OK;
        }, 10);
    }

    void add_client_connection_cancel_await_death_task()
    {
        add_task("Cancel await client connection death", [&] {
            void* connection = pop_client_connection();

            int res = YOGI_CancelAwaitConnectionDeath(connection);

            push_client_connection(connection);
            return res == YOGI_OK;
        }, 3);
    }

    void add_destroy_client_connection_task()
    {
        add_task("Destroy client connection", [&] {
            void* connection = pop_client_connection();

            int res = YOGI_Destroy(connection);
            if (res != YOGI_OK) {
                push_client_connection(connection);
            }

            return res == YOGI_OK;
        }, 30);
    }

    void add_publish_task()
    {
        add_task("Publish", [&] {
            std::lock_guard<std::mutex> lock{serverTerminal.mutex};
            if (serverTerminal.handle) {
                int res = YOGI_PS_Publish(serverTerminal.handle, "Hello",
                    sizeof("Hello"));
                EXPECT_TRUE(res == YOGI_OK || res == YOGI_ERR_NOT_BOUND);
                return res == YOGI_OK;
            }

            return false;
        }, 3);
    }
};

TEST_F(TcpStressTest, LeafLeafConnection)
{
	serverLeaf.sched.handle = helpers::make_scheduler(2);
	serverLeaf.handle       = helpers::make_leaf(serverLeaf.sched.handle);
    serverTerminal.handle   = helpers::make_terminal(serverTerminal.leaf.handle,
        YOGI_TM_PUBLISHSUBSCRIBE, "Server");

	clientLeaf.sched.handle = helpers::make_scheduler(2);
	clientLeaf.handle       = helpers::make_leaf(clientLeaf.sched.handle);
    clientTerminal.handle   = helpers::make_terminal(clientTerminal.leaf.handle,
        YOGI_TM_PUBLISHSUBSCRIBE, "Client");
    helpers::make_binding(clientTerminal.handle, "Server");

    serverScheduler.handle = helpers::make_scheduler();
    clientScheduler.handle = helpers::make_scheduler();

    add_create_server_task();
    add_destroy_object_task("Destroy server", server);
    add_accept_task();
    add_cancel_accept_task();
    add_check_accept_handler_task();
	add_assign_server_connection_task();
    add_server_connection_await_death_task();
    add_server_connection_cancel_await_death_task();
    add_destroy_server_connection_task();
    add_publish_task();

    add_create_client_task();
    add_destroy_object_task("Destroy client", client);
    add_connect_task();
    add_cancel_connect_task();
    add_check_connect_handler_task();
	add_assign_client_connection_task();
    add_client_connection_await_death_task();
    add_client_connection_cancel_await_death_task();
    add_destroy_client_connection_task();

    run(1000);
}

TEST_F(TcpStressTest, ConnectionDestruction)
{
    auto scheduler = helpers::make_scheduler(5);
    auto node      = helpers::make_node(helpers::make_scheduler(1));
    auto leafA     = helpers::make_leaf(helpers::make_scheduler(1));
    auto leafB     = helpers::make_leaf(helpers::make_scheduler(1));
    auto terminalA = helpers::make_terminal(leafA, YOGI_TM_PRODUCER, "T");
    auto terminalB = helpers::make_terminal(leafB, YOGI_TM_CONSUMER, "T");
    auto server    = helpers::make_tcp_server(helpers::make_scheduler(1));
    auto client    = helpers::make_tcp_client(helpers::make_scheduler(1));

    std::atomic<bool> done(false);
    std::atomic<int> totalPub(0);
    std::atomic<int> successfulPub(0);
    std::thread th([&] {
        while (!done) {
            int res = YOGI_PC_Publish(terminalA, "Banana", std::strlen("Banana"));
            totalPub++;
            if (res == YOGI_OK) {
                ++successfulPub;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(25));
        }
    });

    const int n = 2000;
    for (int i = 0; i < n; ++i) {
        printf("\rProgress: %5.2f%%  Successful publish operations: %5.2f%%     ", ((double)i * 100) / n,
            (double)successfulPub.load() / totalPub.load() * 100);
        fflush(stdout);

        auto connectionsA = helpers::connect_tcp(server, leafA, client, node);
        auto connectionsB = helpers::connect_tcp(server, leafB, client, node);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        helpers::destroy(connectionsA.first, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        helpers::destroy(connectionsA.second, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        helpers::destroy(connectionsB.first, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        helpers::destroy(connectionsB.second, 10);
    }

    printf("\n");
    done = true;
    th.join();
}
