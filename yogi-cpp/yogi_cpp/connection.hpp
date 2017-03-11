#ifndef YOGICPP_CONNECTION_HPP
#define YOGICPP_CONNECTION_HPP

#include "endpoint.hpp"
#include "optional.hpp"
#include "internal/async.hpp"

#include <vector>
#include <chrono>


namespace yogi {

class Connection : public Object
{
private:
    const std::string           m_description;
    const std::string           m_remoteVersion;
    const Optional<std::string> m_remoteIdentification;

protected:
    Connection(void* handle);

public:
    const std::string& description() const
    {
        return m_description;
    }

    const std::string& remote_version() const
    {
        return m_remoteVersion;
    }

    const Optional<std::string>& remote_identification() const
    {
        return m_remoteIdentification;
    }
};


class LocalConnection : public Connection
{
public:
    LocalConnection(Endpoint& endpointA, Endpoint& endpointB);
    virtual ~LocalConnection();

    virtual const std::string& class_name() const override;
};


class NonLocalConnection : public Connection
{
protected:
    NonLocalConnection(void* handle);

public:
    void assign(Endpoint& endpoint, std::chrono::milliseconds timeout);
    void async_await_death(std::function<void (const Failure&)> completionHandler);
    void cancel_await_death();
};

} // namespace yogi

#endif // YOGICPP_CONNECTION_HPP
