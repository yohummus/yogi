#ifndef YOGICPP_PROCESS_HPP
#define YOGICPP_PROCESS_HPP

#include "optional.hpp"
#include "timestamp.hpp"
#include "types.hpp"
#include "observers.hpp"
#include "terminals.hpp"

#include <memory>
#include <chrono>
#include <string>
#include <thread>


namespace yogi {

class Configuration;
class Path;
class Scheduler;
class Leaf;
class OperationalCondition;
class Anomaly;
class Error;
class Warning;
class Logger;
class OperationalObserver;

class ProcessInterface final
{
    friend class Anomaly;
    friend class Error;
    friend class Warning;
    friend class OperationalCondition;
    friend class Logger;
    friend class OperationalObserver;

    struct Implementation;

private:
    static std::unique_ptr<Implementation> ms_implementation;

protected:
    static void _set_anomaly(Anomaly* anomaly);
    static void _clear_anomaly(Anomaly* anomaly);
    static void _add_operational_condition(const OperationalCondition* oc);
    static void _remove_operational_condition(const OperationalCondition* oc);
    static void _notify_operational_condition_change(const OperationalCondition* oc);
    static void _on_new_logger_added(const Logger* logger);
    static void _on_max_log_verbosity_set(verbosity newVerbosity, bool isStdout);
    static void _on_log_verbosity_set(const std::string& component, verbosity newVerbosity, bool isStdout);
    static void _publish_log_message(verbosity severity, const char* file, int line, const char* func,
        std::string&& message, Timestamp timestamp, const std::string& threadName, const std::string& component);
    static void _add_operational_observer(OperationalObserver* observer, std::function<void (const Result&, operational_flag)> callbackFn);
    static void _remove_operational_observer(OperationalObserver* observer);

public:
    static ProcessInterface& instance();
    static const Configuration& config();
    static const Path& location();
    static Scheduler& scheduler();
    static Leaf& leaf();
    static operational_flag operational();

    ProcessInterface(const Configuration& config);
    ProcessInterface(int argc, const char* const argv[], bool configFileRequired = false);
    ~ProcessInterface();
};


class Anomaly
{
private:
    const bool                            m_isError;
    const std::string                     m_message;
    std::chrono::steady_clock::time_point m_expirationTime;

protected:
    Anomaly(const std::string& message, bool isError);

public:
    virtual ~Anomaly();

    bool is_error() const
    {
        return m_isError;
    }

    const std::string& message() const
    {
        return m_message;
    }

    std::chrono::steady_clock::time_point expiration_time() const
    {
        return m_expirationTime;
    }

    void set();
    void set(std::chrono::milliseconds duration);
    void clear();
};


class Error : public Anomaly
{
public:
    Error(const std::string& message);
};


class Warning : public Anomaly
{
public:
    Warning(const std::string& message);
};


class OperationalCondition
{
private:
    const std::string m_name;
    bool              m_isSet;

protected:
    OperationalCondition(const std::string& name);
    virtual ~OperationalCondition();

    void _set();
    void _clear();

public:
    const std::string& name() const
    {
        return m_name;
    }

    bool is_met() const
    {
        return m_isSet;
    }
};


class ManualOperationalCondition : public OperationalCondition
{
public:
    ManualOperationalCondition(const std::string& name);

    void set();
    void clear();
};


class Dependency : public OperationalCondition
{
private:
    std::vector<std::unique_ptr<BindingObserver>>      m_bindingObservers;
    std::vector<std::unique_ptr<SubscriptionObserver>> m_subscriptionObservers;

    std::mutex  m_mutex;
    std::size_t m_nonReadyObservers;

private:
    template <typename State>
    void _on_state_changed(State state)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_nonReadyObservers += (state == ESTABLISHED) ? -1 : +1;
        if (m_nonReadyObservers) {
            this->_clear();
        }
        else {
            this->_set();
        }
    }

    template <typename State, typename Observer, typename Object>
    void _add(std::vector<std::unique_ptr<Observer>>* observers, Object& obj)
    {
        observers->emplace_back(std::make_unique<Observer>(obj));
        observers->back()->add([&](auto state) {
            this->_on_state_changed(state);
        });
        observers->back()->start();
    }

    void _add(Binder& binder)
    {
        _add<binding_state>(&m_bindingObservers, binder);
    }

    template <typename ProtoDescription>
    void _add(ScatterGatherTerminal<ProtoDescription>& terminal)
    {
        _add<subscription_state>(&m_subscriptionObservers, terminal);
    }

    template <typename ProtoDescription>
    void _add(ClientTerminal<ProtoDescription>& terminal)
    {
        _add<subscription_state>(&m_subscriptionObservers, terminal);
    }

public:
    template <typename... BindersAndTerminals>
    Dependency(const std::string& name, BindersAndTerminals&... bindersAndTerminals)
    : OperationalCondition(name)
    , m_nonReadyObservers(0)
    {
        auto _ = { (_add(bindersAndTerminals), 0)... };
    }

    virtual ~Dependency()
    {
        m_bindingObservers.clear();
        m_subscriptionObservers.clear();
    }
};


// Has to be created before the connection is established
class ProcessDependency final : Dependency
{
    struct Implementation;

private:
    Implementation* m_impl;

private:
    void _setup(const Path& processPath);

public:
    template <typename... BindersAndSubscribables>
    ProcessDependency(const Path& processPath, BindersAndSubscribables&... bindersAndSubscribable)
    : Dependency(std::string("Terminals for process ") + processPath.to_string() + " available", bindersAndSubscribable...)
    {
        _setup(processPath);
    }

    template <typename... BindersAndSubscribables>
    ProcessDependency(const std::string& processName, BindersAndSubscribables&... bindersAndSubscribable)
    : Dependency(std::string("Terminals for process ") + processName + " available", bindersAndSubscribable...)
    {
        _setup(processName);
    }

    virtual ~ProcessDependency();
};

} // namespace yogi

#endif // YOGICPP_PROCESS_HPP
