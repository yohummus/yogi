#include "../../src/connections/tcp/TcpClient.hpp"
using namespace yogi::interfaces;
using namespace yogi::connections::tcp;

#include "../helpers/Scheduler.hpp"
#include "../mocks/SchedulerMock.hpp"

#include <gmock/gmock.h>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


struct TcpClientTest : public testing::Test
{
    scheduler_ptr scheduler = std::make_shared<helpers::Scheduler>();
    const std::vector<char> clientIdentification{'H', 'e', 'l', 'l', 'o'};

    boost::asio::ip::tcp::acceptor acceptor{scheduler->io_service(),
        boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v6(),
        YOGI_DEFAULT_TCP_PORT}};
    boost::asio::ip::tcp::socket serverSocket{scheduler->io_service()};

    std::atomic<bool>  handlerCalled{false};
    int                handlerErrorCode;
    tcp_connection_ptr handlerConnection;

    TcpClient::connect_handler_fn connectHandlerFn =
        [&](const api::Exception& e, tcp_connection_ptr conn)
    {
        handlerErrorCode  = e.error_code();
        handlerConnection = conn;
        handlerCalled     = true;
    };

    bool wait_for_handler_called(std::chrono::milliseconds duration)
    {
        while (--duration != std::chrono::milliseconds::zero()) {
            if (handlerCalled) {
                return true;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds{1});
        }

        return false;
    }

    void wait_for_handler_called()
    {
        ASSERT_TRUE(wait_for_handler_called(std::chrono::seconds{5}));
    }

    void accept_client()
    {
        acceptor.accept(serverSocket);
    }

    void close_server_connection()
    {
        serverSocket.close();
    }

    void send_from_server(std::vector<char> data)
    {
        boost::asio::write(serverSocket, boost::asio::buffer(data));
    }

    std::vector<char> make_magic_prefix_buffer()
    {
        return std::vector<char>{'Y', 'O', 'G', 'I', ' '};
    }

    void send_magic_prefix_from_server()
    {
        send_from_server(make_magic_prefix_buffer());
    }

    std::vector<char> make_version_buffer(std::string version
        = YOGI_VERSION)
    {
        std::vector<char> data(YOGI_VERSION_INFO_SIZE);
        std::copy(version.begin(), version.end(), data.begin());
        return data;
    }

    static std::string make_compatible_version()
    {
        static std::string version{YOGI_VERSION};
        return version.substr(0, version.find_last_of('.')) + ".999";
    }

    void send_version_from_server(std::string version
        = make_compatible_version())
    {
        send_from_server(make_version_buffer(version));
    }

    std::vector<char> make_identification_buffer(std::vector<char>
        identification)
    {
        const std::size_t n = sizeof(std::uint32_t);
        std::vector<char> data(n + identification.size());
        std::uint32_t nBufSize = htonl(static_cast<unsigned long>(
            identification.size()));
        std::copy(reinterpret_cast<char*>(&nBufSize),
            reinterpret_cast<char*>(&nBufSize) + n, data.begin());
        boost::asio::buffer_copy(boost::asio::buffer(data) + n,
            boost::asio::buffer(identification));

        return data;
    }

    void send_identification_from_server(std::string identification = "")
    {
        send_from_server(make_identification_buffer(std::vector<char>(
            identification.begin(), identification.end())));
    }

    void test_successful_connect(const char* address)
    {
        TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

        client.async_connect(address, YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
            std::chrono::milliseconds::max());
        accept_client();
        send_magic_prefix_from_server();
        send_version_from_server(make_compatible_version());
        send_identification_from_server("Hello World");

        wait_for_handler_called();
        EXPECT_EQ(YOGI_OK, handlerErrorCode);
        ASSERT_TRUE(handlerConnection);
        EXPECT_EQ(make_compatible_version(),
            handlerConnection->remote_version());
        EXPECT_EQ(std::string("Hello World"), std::string(
            handlerConnection->remote_identification().begin(),
            handlerConnection->remote_identification().end()));
    }
};

TEST_F(TcpClientTest, SuccessfulConnectIPv4)
{
    test_successful_connect("127.0.0.1");
}

TEST_F(TcpClientTest, SuccessfulConnectIPv6)
{
    test_successful_connect("::1");
}

TEST_F(TcpClientTest, DataSentToServer)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());
    accept_client();

    std::vector<char> expectedData = {'Y', 'O', 'G', 'I', ' '};
    auto version = make_version_buffer();
    expectedData.insert(expectedData.end(), version.begin(), version.end());
    auto ident = make_identification_buffer(clientIdentification);
    expectedData.insert(expectedData.end(), ident.begin(), ident.end());

    std::vector<char> receivedData(expectedData.size());
    boost::asio::read(serverSocket, boost::asio::buffer(receivedData));
    EXPECT_EQ(expectedData, receivedData);
}

TEST_F(TcpClientTest, ResolveFailed)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("###", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());

    wait_for_handler_called(std::chrono::minutes{1});
    EXPECT_EQ(YOGI_ERR_RESOLVE_FAILED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, ConnectFailed)
{
    acceptor.close();

    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());

    wait_for_handler_called();
    EXPECT_NE(YOGI_OK, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, BrokenSocket)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());
    accept_client();
    close_server_connection();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_SOCKET_BROKEN, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, InvalidMagicPrefix)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());
    accept_client();
    send_from_server(std::vector<char>(10, 'x'));

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_INVALID_MAGIC_PREFIX, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, IncompatibleVersion)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());
    accept_client();
    send_magic_prefix_from_server();
    send_version_from_server("999.999.999");

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_INCOMPATIBLE_VERSION, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, InvalidIdentificationSize)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());
    accept_client();
    send_magic_prefix_from_server();
    send_version_from_server();

    std::uint32_t size = htonl(YOGI_MAX_TCP_IDENTIFICATION_SIZE + 1);
    send_from_server({
        reinterpret_cast<const char*>(&size)[0],
        reinterpret_cast<const char*>(&size)[1],
        reinterpret_cast<const char*>(&size)[2],
        reinterpret_cast<const char*>(&size)[3]
    });

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_IDENTIFICATION_TOO_LARGE, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, ShakeHandsTimeout)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds{5});
    accept_client();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_TIMEOUT, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, CancelWhileResolvingOrConnecting)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());

    client.cancel_connect();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, CancelWhileShakingHands)
{
    TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));

    client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
        std::chrono::milliseconds::max());
    accept_client();
    std::this_thread::sleep_for(std::chrono::milliseconds{5});

    client.cancel_connect();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, DestructWhileResolvingOrConnecting)
{
    {{
        TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));
        client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
            std::chrono::milliseconds::max());
    }}

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpClientTest, DestructWhileShakingHands)
{
    {{
        TcpClient client(*scheduler, boost::asio::buffer(clientIdentification));
        client.async_connect("::1", YOGI_DEFAULT_TCP_PORT, connectHandlerFn,
            std::chrono::milliseconds::max());
        accept_client();
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }}

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

