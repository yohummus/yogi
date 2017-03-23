#include "connection.hpp"
#include "logging.hpp"
#include "internal/utility.hpp"

#include <yogi_core.h>


namespace yogi {
namespace {

std::string get_string(int (*apiFn)(void* conn, char* buf, unsigned bufSize), void* conn)
{
    char buffer[128];
    int res = apiFn(conn, buffer, sizeof(buffer));
    if (res == YOGI_ERR_BUFFER_TOO_SMALL) {
        YOGI_LOG_WARNING(Logger::yogi_logger(), "Could not retrieve string for connection " << conn << ": " << Result(res));
    }
    else if (res != YOGI_OK) {
        YOGI_LOG_ERROR(Logger::yogi_logger(), "Could not retrieve string for connection " << conn << ": " << Result(res));
    }

    return buffer;
}

Optional<std::string> get_identification(void* conn)
{
    std::vector<char> buffer(64);

    unsigned bytesWritten;
    int res = YOGI_GetRemoteIdentification(conn, buffer.data(), buffer.size(), &bytesWritten);
    while (res == YOGI_ERR_BUFFER_TOO_SMALL) {
        buffer.resize(buffer.size() * 2);
        res = YOGI_GetRemoteIdentification(conn, buffer.data(), buffer.size(), &bytesWritten);
    }

    if (res == YOGI_OK) {
        if (bytesWritten == 0) {
            return none;
        }
        else {
            buffer.resize(bytesWritten);
            return std::string(buffer.begin(), buffer.end());
        }
    }

    throw Failure(res);
}

void* make_local_connection(Endpoint& endpointA, Endpoint& endpointB)
{
    void* handle;
    int res = YOGI_CreateLocalConnection(&handle, endpointA.handle(), endpointB.handle());
    internal::throw_on_failure(res);
    return handle;
}

} // anonymous namespace

Connection::Connection(void* handle)
: Object(handle)
, m_description         (get_string(YOGI_GetConnectionDescription, handle))
, m_remoteVersion       (get_string(YOGI_GetRemoteVersion, handle))
, m_remoteIdentification(get_identification(handle))
{
}

LocalConnection::LocalConnection(Endpoint& endpointA, Endpoint& endpointB)
: Connection(make_local_connection(endpointA, endpointB))
{
}

LocalConnection::~LocalConnection()
{
    this->_destroy();
}

const std::string& LocalConnection::class_name() const
{
    static std::string s = "LocalConnection";
    return s;
}

NonLocalConnection::NonLocalConnection(void* handle)
: Connection(handle)
{
}

void NonLocalConnection::assign(Endpoint& endpoint, std::chrono::milliseconds timeout)
{
    int timeout_ = static_cast<int>(timeout == timeout.max() ? -1 : timeout.count());
    int res = YOGI_AssignConnection(handle(), endpoint.handle(), timeout_);
    internal::throw_on_failure(res);
}

void NonLocalConnection::async_await_death(std::function<void (const Failure&)> completionHandler)
{
    internal::async_call([=](const Result& res) {
        completionHandler(static_cast<const Failure&>(res));
    }, [&](auto fn, void* userArg) {
        return YOGI_AsyncAwaitConnectionDeath(this->handle(), fn, userArg);
    });
}

void NonLocalConnection::cancel_await_death()
{
    int res = YOGI_CancelAwaitConnectionDeath(this->handle());
    internal::throw_on_failure(res);
}

} // namespace yogi
