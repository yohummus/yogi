#include "tcp.hpp"
#include "configuration.hpp"
#include "process.hpp"
#include "scheduler.hpp"
#include "leaf.hpp"
#include "logging.hpp"
#include "internal/utility.hpp"

#include <yogi_core.h>

#include <mutex>
#include <condition_variable>
#include <thread>


namespace yogi {

TcpConnection::TcpConnection(void* handle)
: NonLocalConnection(handle)
{
}

TcpConnection::~TcpConnection()
{
    this->_destroy();
}

const std::string& TcpConnection::class_name() const
{
    static std::string s = "TcpConnection";
    return s;
}

TcpClient::TcpClient(Scheduler& scheduler, const Optional<std::string>& identification)
: Object(YOGI_CreateTcpClient, scheduler.handle(), internal::get_raw_string_pointer(identification),
    internal::get_string_size(identification))
, m_scheduler(scheduler)
, m_identification(identification)
{
}

TcpClient::~TcpClient()
{
    this->_destroy();
}

const std::string& TcpClient::class_name() const
{
    static std::string s = "TcpClient";
    return s;
}

void TcpClient::async_connect(const std::string& host, unsigned port, std::chrono::milliseconds handshakeTimeout,
    std::function<void (const Result&, std::unique_ptr<TcpConnection>)> completionHandler)
{
    internal::async_call<void*>([=](const Result& result, void* connection) {
        auto conn = std::unique_ptr<TcpConnection>(result ? new TcpConnection(connection) : nullptr);
        completionHandler(result, std::move(conn));
    }, [&](auto fn, void* userArg) {
        int timeout_ = handshakeTimeout == handshakeTimeout.max() ? -1 : static_cast<int>(handshakeTimeout.count());
        return YOGI_AsyncTcpConnect(this->handle(), host.c_str(), port, timeout_, fn, userArg);
    });
}

void TcpClient::cancel_connect()
{
    int res = YOGI_CancelTcpConnect(this->handle());
    internal::throw_on_failure(res);
}

TcpServer::TcpServer(Scheduler& scheduler, const std::string& address, unsigned port,
    const Optional<std::string>& identification)
: Object(YOGI_CreateTcpServer, scheduler.handle(), internal::get_raw_string_pointer(address), port,
    internal::get_raw_string_pointer(identification), internal::get_string_size(identification))
, m_scheduler(scheduler)
, m_address(address)
, m_port(port)
, m_identification(identification)
{
}

TcpServer::~TcpServer()
{
    this->_destroy();
}

const std::string& TcpServer::class_name() const
{
    static std::string s = "TcpServer";
    return s;
}

void TcpServer::async_accept(std::chrono::milliseconds handshakeTimeout,
    std::function<void (const Result&, std::unique_ptr<TcpConnection>)> completionHandler)
{
    internal::async_call<void*>([=](const Result& result, void* connection) {
        auto conn = std::unique_ptr<TcpConnection>(result ? new TcpConnection(connection) : nullptr);
        completionHandler(result, std::move(conn));
    }, [&](auto fn, void* userArg) {
        int timeout_ = handshakeTimeout == handshakeTimeout.max() ? -1 : static_cast<int>(handshakeTimeout.count());
        return YOGI_AsyncTcpAccept(this->handle(), timeout_, fn, userArg);
    });
}

void TcpServer::cancel_accept()
{
    int res = YOGI_CancelTcpAccept(this->handle());
    internal::throw_on_failure(res);
}

struct AutoConnectingTcpClient::Implementation {
    Endpoint&                      endpoint;
    std::string                    host;
    unsigned                       port;
    std::chrono::milliseconds      timeout;
    std::unique_ptr<TcpClient>     client;
    std::unique_ptr<TcpConnection> connection;
    std::mutex                     mutex;
    std::condition_variable        cv;
    std::thread                    reconnectThread;
    bool                           running;
    bool                           threadInitialised;

    std::function<void (const Result&, const std::unique_ptr<TcpConnection>&)>  connectObserver;
    std::function<void (const Failure&)>                                        disconnectObserver;

    Implementation(Endpoint& ep)
    : endpoint(ep)
    {
    }

    void reconnect_thread_fn()
    {
        std::unique_lock<std::mutex> lock(mutex);
        threadInitialised = true;
        cv.notify_one();

        while (true) {
            cv.wait(lock);
            if (!running) {
                return;
            }

            connection.reset();

            cv.wait_for(lock, std::chrono::seconds(1));
            if (!running) {
                return;
            }

            start_connect();
        }
    }

    void start_connect()
    {
        YOGI_LOG_INFO(Logger::yogi_logger(), "Connecting to " << host << ":" << port << "...");

        client->async_connect(host, port, timeout, [&](auto& res, auto conn) {
            this->on_connect_completed(res, std::move(conn));
        });
    }

    void on_connect_completed(const Result& res, std::unique_ptr<TcpConnection> conn)
    {
        if (res == YOGI_ERR_CANCELED) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex);
        if (!running) {
            return;
        }

        Failure err(YOGI_ERR_UNKNOWN);
        if (res == Success()) {
            try {
                conn->assign(endpoint, timeout);
                conn->async_await_death([&](const Failure& err) {
                    this->on_connection_died(err);
                });
                connection = std::move(conn);

                YOGI_LOG_INFO(Logger::yogi_logger(), "Connection to " << host << ":" << port << " established successfully");

                if (connectObserver) {
                    connectObserver(Success(), connection);
                }

                return;
            }
            catch (const Failure& err_) {
                err = err_;
            }
        }
        else {
            err = Failure(res.value());
        }

        YOGI_LOG_DEBUG(Logger::yogi_logger(), "Could not connect to " << host << ":" << port << ": " << err);

        if (connectObserver) {
            connectObserver(err, std::unique_ptr<TcpConnection>());
        }

        conn.reset();
        cv.notify_one();
    }

    void on_connection_died(const Failure& err)
    {
        if (err == YOGI_ERR_CANCELED) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex);
        if (!running) {
            return;
        }

        YOGI_LOG_WARNING(Logger::yogi_logger(), "Connection to " << host << ":" << port << " lost: " << err);

        if (disconnectObserver) {
            disconnectObserver(err);
        }

        cv.notify_one();
    }
};

AutoConnectingTcpClient::AutoConnectingTcpClient(Endpoint& endpoint, const std::string& host, unsigned port,
    std::chrono::milliseconds timeout, const Optional<std::string>& identification)
: m_impl(std::make_unique<Implementation>(endpoint))
{
    m_impl->host              = host;
    m_impl->port              = port;
    m_impl->timeout           = timeout;
    m_impl->running           = false;
    m_impl->threadInitialised = false;
    m_impl->client            = std::make_unique<TcpClient>(endpoint.scheduler(), identification);

    m_impl->reconnectThread = std::thread(&Implementation::reconnect_thread_fn, &*m_impl);
}

AutoConnectingTcpClient::AutoConnectingTcpClient(Endpoint& endpoint, const Configuration& config)
: AutoConnectingTcpClient(endpoint, internal::tcp_target_to_host(config.connection_target()),
    internal::tcp_target_to_port(config.connection_target()), config.connection_timeout(),
    config.connection_identification())
{
}

AutoConnectingTcpClient::AutoConnectingTcpClient(ProcessInterface& pi)
: AutoConnectingTcpClient(pi.leaf(), pi.config())
{
}

AutoConnectingTcpClient::~AutoConnectingTcpClient()
{
    {{
    std::unique_lock<std::mutex> lock(m_impl->mutex);
    m_impl->running = false;
    m_impl->cv.wait(lock, [&] {
        return m_impl->threadInitialised;
    });

    m_impl->cv.notify_one();
    }}

    m_impl->reconnectThread.join();

    m_impl->client.reset();
}

Endpoint& AutoConnectingTcpClient::endpoint()
{
    return m_impl->endpoint;
}

const std::string& AutoConnectingTcpClient::host() const
{
    return m_impl->host;
}

unsigned AutoConnectingTcpClient::port() const
{
    return m_impl->port;
}

std::chrono::milliseconds AutoConnectingTcpClient::timeout() const
{
    return m_impl->timeout;
}

const Optional<std::string>& AutoConnectingTcpClient::identification() const
{
    return m_impl->client->identification();
}

void AutoConnectingTcpClient::start()
{
    std::unique_lock<std::mutex> lock(m_impl->mutex);
    if (m_impl->running) {
        throw AlreadyStarted();
    }
    if (m_impl->host.empty() || m_impl->port == 0 || m_impl->port > 65535) {
        throw InvalidTarget();
    }

    m_impl->cv.wait(lock, [&] {
        return m_impl->threadInitialised;
    });

    m_impl->start_connect();
    m_impl->running = true;
}

bool AutoConnectingTcpClient::try_start()
{
    try {
        start();
        return true;
    }
    catch (...) {
        return false;
    }
}

void AutoConnectingTcpClient::set_connect_observer(std::function<void (const Result&, const std::unique_ptr<TcpConnection>&)> fn)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->connectObserver = fn;
}

std::function<void (const Result&, const std::unique_ptr<TcpConnection>&)> AutoConnectingTcpClient::connect_observer() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->connectObserver;
}

void AutoConnectingTcpClient::set_disconnect_observer(std::function<void (const Failure&)> fn)
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->disconnectObserver = fn;
}

std::function<void (const Failure&)> AutoConnectingTcpClient::disconnect_observer() const
{
    std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->disconnectObserver;
}

} // namespace yogi
