#include "../../src/connections/tcp/TcpServer.hpp"
using namespace yogi::api;
using namespace yogi::interfaces;
using namespace yogi::connections::tcp;

#include "../helpers/Scheduler.hpp"

#include <gmock/gmock.h>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


struct TcpServerTest : public testing::Test
{
    scheduler_ptr scheduler = std::make_shared<helpers::Scheduler>();
    const std::vector<char> serverIdentification{'H', 'e', 'l', 'l', 'o'};

    boost::asio::ip::tcp::socket clientSocket{scheduler->io_service()};

    std::atomic<bool>  handlerCalled{false};
    int                handlerErrorCode;
    tcp_connection_ptr handlerConnection;

    TcpServer::accept_handler_fn acceptHandlerFn =
        [&](const Exception& e, tcp_connection_ptr conn)
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

    void connect_client(const char* address = "::1")
    {
        clientSocket.connect(boost::asio::ip::tcp::endpoint(
            boost::asio::ip::address::from_string(address),
            YOGI_DEFAULT_TCP_PORT));
    }

    void close_client_connection()
    {
        clientSocket.close();
    }

    void send_from_client(std::vector<char> data)
    {
        boost::asio::write(clientSocket, boost::asio::buffer(data));
    }

    std::vector<char> make_magic_prefix_buffer()
    {
        return std::vector<char>{'C', 'H', 'I', 'R', 'P', ' '};
    }

    void send_magic_prefix_from_client()
    {
        send_from_client(make_magic_prefix_buffer());
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

    void send_version_from_client(std::string version
        = make_compatible_version())
    {
        send_from_client(make_version_buffer(version));
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

    void send_identification_from_client(std::string identification = "")
    {
        send_from_client(make_identification_buffer(std::vector<char>(
            identification.begin(), identification.end())));
    }

    void test_successful_accept(const char* address)
    {
        TcpServer server(*scheduler, address, YOGI_DEFAULT_TCP_PORT,
            boost::asio::buffer(serverIdentification));

        server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
        connect_client(address);
        send_magic_prefix_from_client();
        send_version_from_client(make_compatible_version());
        send_identification_from_client("Hello World");

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

TEST_F(TcpServerTest, SuccessfulAcceptIPv4)
{
    test_successful_accept("127.0.0.1");
}

TEST_F(TcpServerTest, SuccessfulAcceptIPv6)
{
    test_successful_accept("::1");
}

TEST_F(TcpServerTest, DataSentToClient)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    connect_client();

    std::vector<char> expectedData = {'C', 'H', 'I', 'R', 'P', ' '};
    auto version = make_version_buffer();
    expectedData.insert(expectedData.end(), version.begin(), version.end());
    auto ident = make_identification_buffer(serverIdentification);
    expectedData.insert(expectedData.end(), ident.begin(), ident.end());

    std::vector<char> receivedData(expectedData.size());
    boost::asio::read(clientSocket, boost::asio::buffer(receivedData));
    EXPECT_EQ(expectedData, receivedData);
}

TEST_F(TcpServerTest, BrokenSocket)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    connect_client();
    close_client_connection();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_SOCKET_BROKEN, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, InvalidMagicPrefix)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    connect_client();
    send_from_client(std::vector<char>(10, 'x'));

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_INVALID_MAGIC_PREFIX, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, IncompatibleVersion)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    connect_client();
    send_magic_prefix_from_client();
    send_version_from_client("999.999.999");

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_INCOMPATIBLE_VERSION, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, InvalidIdentificationSize)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    connect_client();
    send_magic_prefix_from_client();
    send_version_from_client();

    std::uint32_t size = htonl(YOGI_MAX_TCP_IDENTIFICATION_SIZE + 1);
    send_from_client({
        reinterpret_cast<const char*>(&size)[0],
        reinterpret_cast<const char*>(&size)[1],
        reinterpret_cast<const char*>(&size)[2],
        reinterpret_cast<const char*>(&size)[3]
    });

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_IDENTIFICATION_TOO_LARGE, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, ShakeHandsTimeout)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds{5});
    connect_client();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_TIMEOUT, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, CancelWhileAcceptingConnection)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    server.cancel_accept();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, CancelWhileShakingHands)
{
    TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
        boost::asio::buffer(serverIdentification));

    server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    connect_client();
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
    server.cancel_accept();

    wait_for_handler_called();
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, DestructWhileAcceptingConnection)
{
    {{
        TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
            boost::asio::buffer(serverIdentification));

        server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
    }}

    EXPECT_TRUE(handlerCalled);
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

TEST_F(TcpServerTest, DestructWhileShakingHands)
{
    {{
        TcpServer server(*scheduler, "::1", YOGI_DEFAULT_TCP_PORT,
            boost::asio::buffer(serverIdentification));

        server.async_accept(acceptHandlerFn, std::chrono::milliseconds::max());
        connect_client();
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }}

    EXPECT_TRUE(handlerCalled);
    EXPECT_EQ(YOGI_ERR_CANCELED, handlerErrorCode);
    EXPECT_EQ(tcp_connection_ptr{}, handlerConnection);
}

