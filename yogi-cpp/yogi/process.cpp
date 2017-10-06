#include "process.hpp"
#include "path.hpp"
#include "configuration.hpp"
#include "scheduler.hpp"
#include "leaf.hpp"
#include "terminals.hpp"
#include "logging.hpp"
#include "internal/proto/yogi_00000001.h"
#include "internal/proto/yogi_00004004.h"
#include "internal/proto/yogi_0000040d.h"
#include "internal/proto/yogi_000009cd.h"

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>
#include <sstream>
#include <memory>
using namespace std::string_literals;


namespace yogi {

struct ProcessInterface::Implementation {
    ProcessInterface*                           instance;
    Configuration                               config;
    Scheduler                                   scheduler;
    Leaf                                        leaf;
    bool                                        running;

    CachedProducerTerminal<internal::yogi_00000001> operationalTerminal;
    CachedProducerTerminal<internal::yogi_0000040d> errorsTerminal;
    CachedProducerTerminal<internal::yogi_0000040d> warningsTerminal;
    ProducerTerminal<internal::yogi_000009cd>       logTerminal;

    std::mutex                                                                            logVerbosityTerminalsMutex;
    CachedMasterTerminal<internal::yogi_00004004>                                         maxStdoutLogVerbosityTerminal;
    CachedMasterTerminal<internal::yogi_00004004>                                         maxYogiLogVerbosityTerminal;
    std::map<std::string, std::unique_ptr<CachedMasterTerminal<internal::yogi_00004004>>> stdoutLogVerbosityTerminals;
    std::map<std::string, std::unique_ptr<CachedMasterTerminal<internal::yogi_00004004>>> yogiLogVerbosityTerminals;

    std::mutex                  anomaliesMutex;
    std::condition_variable     anomaliesCv;
    std::thread                 anomaliesThread;
    std::vector<const Anomaly*> activeAnomalies;

    std::mutex                                                                            operationalMutex;
    std::vector<const OperationalCondition*>                                              operationalConditions;
    operational_flag                                                                      operationalState;
    std::map<OperationalObserver*, std::function<void (const Result&, operational_flag)>> operationalObservers;

    Implementation(const Configuration& config)
    : leaf(scheduler)
    , operationalTerminal          (leaf, config.location() / "Process/Operational")
    , errorsTerminal               (leaf, config.location() / "Process/Errors")
    , warningsTerminal             (leaf, config.location() / "Process/Warnings")
    , logTerminal                  (leaf, config.location() / "Process/Log")
    , maxStdoutLogVerbosityTerminal(leaf, config.location() / "Process/Standard Output Log Verbosity/Max Verbosity")
    , maxYogiLogVerbosityTerminal  (leaf, config.location() / "Process/YOGI Log Verbosity/Max Verbosity")
    {
    }

    void anomalies_thread_fn()
    {
        std::unique_lock<std::mutex> lock(anomaliesMutex);
        while (running) {
            auto nextExpiration = update_active_anomalies();
            if (nextExpiration == nextExpiration.max()) {
                anomaliesCv.wait(lock);
            }
            else {
                anomaliesCv.wait_until(lock, nextExpiration);
            }
        }
    }

    std::chrono::steady_clock::time_point update_active_anomalies()
    {
        auto now = std::chrono::steady_clock::now();
        auto nextExpirationTime = std::chrono::steady_clock::time_point::max();

        bool updateErrors   = false;
        bool updateWarnings = false;

        auto& aas = activeAnomalies;
        for (auto it = aas.begin(); it != aas.end(); ) {
            auto anomaly = *it;
            if (anomaly->expiration_time() > now) {
                nextExpirationTime = std::min(nextExpirationTime, anomaly->expiration_time());
                ++it;
            }
            else {
                if (anomaly->is_error()) {
                    updateErrors = true;
                }
                else {
                    updateWarnings = true;
                }

                YOGI_LOG_INFO(Logger::yogi_logger(), (anomaly->is_error() ? "Error" : "Warning")
                    << " '" << anomaly->message() << "' expired");
                it = aas.erase(it);
            }
        }

        if (updateErrors) {
            publish_anomalies(true);
        }

        if (updateWarnings) {
            publish_anomalies(false);
        }

        return nextExpirationTime;
    }

    void publish_anomalies(bool errors)
    {
        auto msg = errorsTerminal.make_message();

        for (auto anomaly : activeAnomalies) {
            if (anomaly->is_error() == errors) {
                *msg.add_value() = anomaly->message();
            }
        }

        if (errors) {
            errorsTerminal.try_publish(msg);
        }
        else {
            warningsTerminal.try_publish(msg);
        }
    }

    bool update_operational_state()
    {
        operational_flag operational = true;
        for (auto oc : operationalConditions) {
            if (!oc->is_met()) {
                operational = false;
                break;
            }
        }

        if (operational != operationalState) {
            operationalState = operational;
            publish_operational_state(operational);

            for (auto& entry : operationalObservers) {
                entry.second(Success(), operational);
            }

            return true;
        }

        return false;
    }

    void publish_operational_state(operational_flag operational)
    {
        auto msg = operationalTerminal.make_message();
        msg.set_value(operational);
        operationalTerminal.try_publish(msg);
    }

    verbosity get_verbosity_from_config(Optional<ConfigurationChild> child)
    {
        if (!child) {
            return verbosity::TRACE;
        }

        auto str = child->get_value<std::string>();
        boost::to_upper(str);

        if (str == "TRACE") {
            return verbosity::TRACE;
        }
        else if (str == "DEBUG") {
            return verbosity::DEBUG;
        }
        else if (str == "INFO") {
            return verbosity::INFO;
        }
        else if (str == "WARNING") {
            return verbosity::WARNING;
        }
        else if (str == "ERROR") {
            return verbosity::ERROR;
        }
        else if (str == "FATAL") {
            return verbosity::FATAL;
        }
        else {
            throw std::runtime_error("Invalid logging verbosity '"s + child->get_value<std::string>()
                + "' (valid values: TRACE, DEBUG, INFO, WARNING, ERROR, FATAL)");
        }
    }

    void apply_verbosities(const std::string& configPath, void (*setMaxVerbosityFn)(verbosity), void (Logger::*setVerbosityFn)(verbosity))
    {
        auto child = config.get_child_optional(configPath);
        if (child) {
            setMaxVerbosityFn(get_verbosity_from_config(child->get_child_optional("max-verbosity")));
            auto componentVerbosityChild = child->get_child_optional("component-verbosity");
            if (componentVerbosityChild) {
                for (auto entry : *componentVerbosityChild) {
                    Logger logger(entry.first);
                    (logger.*setVerbosityFn)(get_verbosity_from_config(entry.second));
                }
            }
        }
    }

    void initialise_logging()
    {
        Logger::set_colourised_stdout(config.get<bool>("logging.stdout.colourised", false));

        auto loggers = Logger::get_all_loggers();
        for (auto& logger : loggers) {
            register_logger(logger);
        }

        apply_verbosities("logging.stdout", &Logger::set_max_stdout_verbosity, &Logger::set_stdout_verbosity);
        apply_verbosities("logging.yogi",  &Logger::set_max_yogi_verbosity,  &Logger::set_yogi_verbosity);
    }

    void on_max_logging_verbosity_changed(const Result& res, internal::yogi_00004004::SlaveMessage&& msg, cached_flag cached, bool isStdout)
    {
        if (!res) {
            return;
        }

        verbosity verb = msg.value() <= static_cast<int>(verbosity::TRACE)
            ? static_cast<verbosity>(msg.value())
            : verbosity::TRACE;

        if (isStdout) {
            if (!cached) {
                Logger::set_max_stdout_verbosity(verb);
            }

            maxStdoutLogVerbosityTerminal.async_receive_message([=](auto& res, auto&& msg, auto cached) {
                this->on_max_logging_verbosity_changed(res, std::move(msg), cached, true);
            });
        }
        else {
            if (!cached) {
                Logger::set_max_yogi_verbosity(verb);
            }

            maxYogiLogVerbosityTerminal.async_receive_message([=](auto& res, auto&& msg, auto cached) {
                this->on_max_logging_verbosity_changed(res, std::move(msg), cached, false);
            });
        }
    }

    void monitor_max_logging_verbosity_terminals()
    {
        maxStdoutLogVerbosityTerminal.async_receive_message([=](auto& res, auto&& msg, auto cached) {
            this->on_max_logging_verbosity_changed(res, std::move(msg), cached, true);
        });

        maxYogiLogVerbosityTerminal.async_receive_message([=](auto& res, auto&& msg, auto cached) {
            this->on_max_logging_verbosity_changed(res, std::move(msg), cached, false);
        });
    }

    void on_logging_verbosity_changed(const Result& res, internal::yogi_00004004::SlaveMessage&& msg, cached_flag cached, bool isStdout, const std::string& component)
    {
        if (!res) {
            return;
        }

        Logger logger(component);

        verbosity verb = msg.value() <= static_cast<int>(verbosity::TRACE)
            ? static_cast<verbosity>(msg.value())
            : verbosity::TRACE;

        if (isStdout) {
            if (!cached) {
                logger.set_stdout_verbosity(verb);
            }

            std::lock_guard<std::mutex> lock(logVerbosityTerminalsMutex);
            stdoutLogVerbosityTerminals[component]->async_receive_message([=](auto& res, auto&& msg, auto cached) {
                this->on_logging_verbosity_changed(res, std::move(msg), cached, true, component);
            });
        }
        else {
            if (!cached) {
                logger.set_yogi_verbosity(verb);
            }

            std::lock_guard<std::mutex> lock(logVerbosityTerminalsMutex);
            yogiLogVerbosityTerminals[component]->async_receive_message([=](auto& res, auto&& msg, auto cached) {
                this->on_logging_verbosity_changed(res, std::move(msg), cached, false, component);
            });
        }
    }

    void register_logger(const Logger& logger)
    {
        auto component = logger.component();

        auto stdoutVerbosityTerminal = std::make_unique<CachedMasterTerminal<internal::yogi_00004004>>(leaf,
            config.location() / "Process/Standard Output Log Verbosity/Components/" / logger.component());
        stdoutVerbosityTerminal->async_receive_message([=](auto& res, auto&& msg, auto cached) {
            this->on_logging_verbosity_changed(res, std::move(msg), cached, true, component);
        });

        auto yogiVerbosityTerminal = std::make_unique<CachedMasterTerminal<internal::yogi_00004004>>(leaf,
            config.location() / "Process/YOGI Log Verbosity/Components/" / logger.component());
        yogiVerbosityTerminal->async_receive_message([=](auto& res, auto&& msg, auto cached) {
            this->on_logging_verbosity_changed(res, std::move(msg), cached, false, component);
        });

        std::lock_guard<std::mutex> lock(logVerbosityTerminalsMutex);
        stdoutLogVerbosityTerminals[component] = std::move(stdoutVerbosityTerminal);
        yogiLogVerbosityTerminals[component]  = std::move(yogiVerbosityTerminal);
    }
};

std::unique_ptr<ProcessInterface::Implementation> ProcessInterface::ms_implementation;

void ProcessInterface::_set_anomaly(Anomaly* anomaly)
{
    std::lock_guard<std::mutex> lock(ms_implementation->anomaliesMutex);

    auto& aas = ms_implementation->activeAnomalies;
    if (std::find(aas.begin(), aas.end(), anomaly) == aas.end()) {
        aas.push_back(anomaly);
        if (anomaly->is_error()) {
            YOGI_LOG_ERROR(Logger::yogi_logger(), "Error '" << anomaly->message() << "' set");
        }
        else {
            YOGI_LOG_WARNING(Logger::yogi_logger(), "Warning '" << anomaly->message() << "' set");
        }
        ms_implementation->publish_anomalies(anomaly->is_error());
    }

    ms_implementation->anomaliesCv.notify_one();
}

void ProcessInterface::_clear_anomaly(Anomaly* anomaly)
{
    std::lock_guard<std::mutex> lock(ms_implementation->anomaliesMutex);

    auto& aas = ms_implementation->activeAnomalies;
    auto it = std::find(aas.begin(), aas.end(), anomaly);
    if (it != aas.end()) {
        aas.erase(it);
        YOGI_LOG_INFO(Logger::yogi_logger(), (anomaly->is_error() ? "Error" : "Warning")
            << " '" << anomaly->message() << "' cleared");
        ms_implementation->publish_anomalies(anomaly->is_error());
        ms_implementation->anomaliesCv.notify_one();
    }
}

void ProcessInterface::_add_operational_condition(const OperationalCondition* oc)
{
    std::lock_guard<std::mutex> lock(ms_implementation->operationalMutex);
    ms_implementation->operationalConditions.push_back(oc);

    bool changed = ms_implementation->update_operational_state();
    if (changed) {
        YOGI_LOG_INFO(Logger::yogi_logger(), "Operational state changed to "
            << (ms_implementation->operationalState ? "TRUE" : "FALSE")
            << " due to the addition of the Operational Condition '" << oc->name() << "'");
    }
}

void ProcessInterface::_remove_operational_condition(const OperationalCondition* oc)
{
    auto& ocs = ms_implementation->operationalConditions;

    std::lock_guard<std::mutex> lock(ms_implementation->operationalMutex);
    ocs.erase(std::find(ocs.begin(), ocs.end(), oc));

    bool changed = ms_implementation->update_operational_state();
    if (changed) {
        YOGI_LOG_INFO(Logger::yogi_logger(), "Operational state changed to "
            << (ms_implementation->operationalState ? "TRUE" : "FALSE")
            << " due to the removal of the Operational Condition '" << oc->name() << "'");
    }
}

void ProcessInterface::_notify_operational_condition_change(const OperationalCondition* oc)
{
    std::lock_guard<std::mutex> lock(ms_implementation->operationalMutex);
    bool changed = ms_implementation->update_operational_state();
    if (changed) {
        YOGI_LOG_INFO(Logger::yogi_logger(), "Operational state changed to "
            << (ms_implementation->operationalState ? "TRUE" : "FALSE")
            << " due to a change of the Operational Condition '" << oc->name() << "'");
    }
}

void ProcessInterface::_on_new_logger_added(const Logger* logger)
{
    if (ms_implementation) {
        ms_implementation->register_logger(*logger);
    }
}

void ProcessInterface::_on_max_log_verbosity_set(verbosity newVerbosity, bool isStdout)
{
    if (!ms_implementation) {
        return;
    }

    internal::yogi_00004004::MasterMessage msg;
    msg.set_value(static_cast<std::uint8_t>(newVerbosity));

    if (isStdout) {
        ms_implementation->maxStdoutLogVerbosityTerminal.try_publish(msg);
    }
    else {
        ms_implementation->maxYogiLogVerbosityTerminal.try_publish(msg);
    }
}

void ProcessInterface::_on_log_verbosity_set(const std::string& component, verbosity newVerbosity, bool isStdout)
{
    if (!ms_implementation) {
        return;
    }

    internal::yogi_00004004::MasterMessage msg;
    msg.set_value(static_cast<std::uint8_t>(newVerbosity));

    if (isStdout) {
        ms_implementation->stdoutLogVerbosityTerminals[component]->try_publish(msg);
    }
    else {
        ms_implementation->yogiLogVerbosityTerminals[component]->try_publish(msg);
    }
}

void ProcessInterface::_publish_log_message(verbosity severity, const char* file, int line, const char* func,
    std::string&& message, Timestamp timestamp, const std::string& threadName, const std::string& component)
{
    if (!ms_implementation) {
        return;
    }

    boost::replace_all(message, "\"", "\\\"");

    std::stringstream ss;
    ss << "{"
        << "\"severity\":\"" << severity
        << "\",\"thread\":\"" << threadName
        << "\",\"file\":\"" << file
        << "\",\"line\":" << line
        << ",\"function\":\"" << func
        << "\",\"component\":\"" << component
        << "\"}";

    auto& tm = ms_implementation->logTerminal;

    auto msg = tm.make_message();
    decltype(msg)::Pair pair;

    pair.set_first(std::move(message));
    pair.set_second(ss.str());
    msg.set_timestamp(static_cast<std::uint64_t>(timestamp.time_since_epoch().count()));
    msg.set_allocated_value(&pair);

    tm.try_publish(msg);

    msg.release_value();
}

void ProcessInterface::_add_operational_observer(OperationalObserver* observer, std::function<void (const Result&, operational_flag)> callbackFn)
{
    std::lock_guard<std::mutex> lock(ms_implementation->operationalMutex);
    ms_implementation->operationalObservers[observer] = callbackFn;
    callbackFn(Success(), ms_implementation->operationalState);
}

void ProcessInterface::_remove_operational_observer(OperationalObserver* observer)
{
    std::lock_guard<std::mutex> lock(ms_implementation->operationalMutex);
    ms_implementation->operationalObservers.erase(observer);
}

ProcessInterface& ProcessInterface::instance()
{
    assert (!!ms_implementation);
    return *ms_implementation->instance;
}

const Configuration& ProcessInterface::config()
{
    return ms_implementation->config;
}

const Path& ProcessInterface::location()
{
    return ms_implementation->config.location();
}

Scheduler& ProcessInterface::scheduler()
{
    return ms_implementation->scheduler;
}

Leaf& ProcessInterface::leaf()
{
    return ms_implementation->leaf;
}

operational_flag ProcessInterface::operational()
{
    std::lock_guard<std::mutex> lock(ms_implementation->operationalMutex);
    return ms_implementation->operationalState;
}

ProcessInterface::ProcessInterface(const Configuration& config)
{
    assert(!ms_implementation);

    try {
        ms_implementation = std::make_unique<Implementation>(config);
        ms_implementation->config = config;

        ms_implementation->initialise_logging();
        ms_implementation->monitor_max_logging_verbosity_terminals();

        ms_implementation->running = true;
        ms_implementation->anomaliesThread = std::thread(&Implementation::anomalies_thread_fn, &*ms_implementation);

        ms_implementation->publish_anomalies(true);
        ms_implementation->publish_anomalies(false);

        ms_implementation->operationalState = true;
        ms_implementation->publish_operational_state(true);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::exit(1);
    }
}

ProcessInterface::ProcessInterface(int argc, const char* const argv[], bool configFileRequired)
try: ProcessInterface(Configuration(argc, argv, configFileRequired))
{
}
catch (const std::exception& e)
{
    std::cerr << "ERROR: " << e.what() << std::endl;
    std::exit(1);
}

ProcessInterface::~ProcessInterface()
{
    assert(!!ms_implementation);

    {{
    std::lock_guard<std::mutex> lock(ms_implementation->anomaliesMutex);
    ms_implementation->running = false;
    ms_implementation->anomaliesCv.notify_one();
    }}
    ms_implementation->anomaliesThread.join();

    ms_implementation.reset();
}

Anomaly::Anomaly(const std::string& message, bool isError)
: m_isError(isError)
, m_message(message)
{
}

Anomaly::~Anomaly()
{
    clear();
}

void Anomaly::set()
{
    m_expirationTime = std::chrono::steady_clock::time_point::max();
    ProcessInterface::_set_anomaly(this);
}

void Anomaly::set(std::chrono::milliseconds duration)
{
    m_expirationTime = std::chrono::steady_clock::now() + duration;
    ProcessInterface::_set_anomaly(this);
}

void Anomaly::clear()
{
    ProcessInterface::_clear_anomaly(this);
}

Error::Error(const std::string& message)
: Anomaly(message, true)
{
}

Warning::Warning(const std::string& message)
: Anomaly(message, false)
{
}

OperationalCondition::OperationalCondition(const std::string& name)
: m_name(name)
, m_isSet(false)
{
    ProcessInterface::_add_operational_condition(this);
    YOGI_LOG_DEBUG(Logger::yogi_logger(), "Operational Condition '" << m_name << "' added");
}

OperationalCondition::~OperationalCondition()
{
    ProcessInterface::_remove_operational_condition(this);
    YOGI_LOG_DEBUG(Logger::yogi_logger(), "Operational Condition '" << m_name << "' removed");
}

void OperationalCondition::_set()
{
    if (!m_isSet) {
        m_isSet = true;
        ProcessInterface::_notify_operational_condition_change(this);
        YOGI_LOG_DEBUG(Logger::yogi_logger(), "Operational Condition '" << m_name << "' set");
    }
}

void OperationalCondition::_clear()
{
    if (m_isSet) {
        m_isSet = false;
        ProcessInterface::_notify_operational_condition_change(this);
        YOGI_LOG_DEBUG(Logger::yogi_logger(), "Operational Condition '" << m_name << "' cleared");
    }
}

ManualOperationalCondition::ManualOperationalCondition(const std::string& name)
: OperationalCondition(name)
{
}

void ManualOperationalCondition::set()
{
    this->_set();
}

void ManualOperationalCondition::clear()
{
    this->_clear();
}

struct ProcessDependency::Implementation
{
    ManualOperationalCondition                                           poc;
    CachedConsumerTerminal<internal::yogi_00000001>                  operationalTerminal;
    MessageObserver<CachedConsumerTerminal<internal::yogi_00000001>> operationalObserver;

    Implementation(const Path& processPath)
    : poc(std::string("Process ") + processPath.to_string() + " operational")
    , operationalTerminal(processPath / "Process/Operational")
    , operationalObserver(operationalTerminal)
    {
    }

    void updatePoc(const decltype(operationalTerminal)::message_type& msg)
    {
        if (msg.value()) {
            poc.set();
        }
        else {
            poc.clear();
        }
    }
};

void ProcessDependency::_setup(const Path& processPath)
{
    m_impl = new Implementation(processPath);

    m_impl->operationalObserver.add([&](auto& msg) {
        this->m_impl->updatePoc(msg);
    });

    try {
        auto msg = m_impl->operationalTerminal.get_cached_message();
        m_impl->updatePoc(msg);
    }
    catch (...) {
    }

    m_impl->operationalObserver.start();
}

ProcessDependency::~ProcessDependency()
{
    m_impl->operationalObserver.stop();
    delete m_impl;
}

} // namespace yogi
