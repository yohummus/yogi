#ifndef YOGI_TCP_HPP
#define YOGI_TCP_HPP

#include "endpoint.hpp"
#include "result.hpp"
#include "connection.hpp"

#include <exception>


namespace yogi {

class Configuration;
class ProcessInterface;


class TcpConnection : public NonLocalConnection
{
    friend class TcpClient;
    friend class TcpServer;

protected:
    TcpConnection(void* handle);

public:
    virtual ~TcpConnection();
    virtual const std::string& class_name() const override;
};


class TcpClient : public Object
{
private:
    Scheduler&            m_scheduler;
    Optional<std::string> m_identification;

public:
    TcpClient(Scheduler& scheduler, const Optional<std::string>& identification = none);
    virtual ~TcpClient();

    virtual const std::string& class_name() const override;

    Scheduler& scheduler()
    {
        return m_scheduler;
    }

    const Optional<std::string>& identification() const
    {
        return m_identification;
    }

    void async_connect(const std::string& host, unsigned port, std::chrono::milliseconds handshakeTimeout,
        std::function<void (const Result&, std::unique_ptr<TcpConnection>)> completionHandler);
    void cancel_connect();
};


class TcpServer : public Object
{
private:
    Scheduler&            m_scheduler;
    std::string           m_address;
    unsigned              m_port;
    Optional<std::string> m_identification;

public:
    TcpServer(Scheduler& scheduler, const std::string& address, unsigned port,
        const Optional<std::string>& identification = none);
    virtual ~TcpServer();

    virtual const std::string& class_name() const override;

    Scheduler& scheduler()
    {
        return m_scheduler;
    }

    const std::string& address() const
    {
        return m_address;
    }

    unsigned port() const
    {
        return m_port;
    }

    const Optional<std::string>& identification() const
    {
        return m_identification;
    }

    void async_accept(std::chrono::milliseconds handshakeTimeout,
        std::function<void (const Result&, std::unique_ptr<TcpConnection>)> completionHandler);
    void cancel_accept();
};


class InvalidTarget : public std::exception
{
public:
    virtual const char* what() const noexcept override;
};


class AlreadyStarted : public std::exception
{
public:
    virtual const char* what() const noexcept override;
};


class NotRunning : public std::exception
{
public:
    virtual const char* what() const noexcept override;
};


class AutoConnectingTcpClient
{
    struct Implementation;

private:
    std::unique_ptr<Implementation> m_impl;

public:
    AutoConnectingTcpClient(Endpoint& endpoint, const std::string& host, unsigned port,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max(),
        const Optional<std::string>& identification = none);

    AutoConnectingTcpClient(Endpoint& endpoint, const Configuration& config);
    AutoConnectingTcpClient(ProcessInterface& pi);
    virtual ~AutoConnectingTcpClient();

    Endpoint& endpoint();
    const std::string& host() const;
    unsigned port() const;
    std::chrono::milliseconds timeout() const;
    const Optional<std::string>& identification() const;

    void start();
    bool try_start();

    void set_connect_observer(std::function<void (const Result&, const std::unique_ptr<TcpConnection>&)> fn);
    std::function<void (const Result&, const std::unique_ptr<TcpConnection>&)> connect_observer() const;
    void set_disconnect_observer(std::function<void (const Failure&)> fn);
    std::function<void (const Failure&)> disconnect_observer() const;
};

} // namespace yogi

#endif // YOGI_TCP_HPP
