#ifndef YOGI_LOGGING_HPP
#define YOGI_LOGGING_HPP

#include "types.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <mutex>

#define YOGI_LOG_FATAL(...) \
    YOGI_LOG(FATAL, __VA_ARGS__)

#define YOGI_LOG_ERROR(...) \
    YOGI_LOG(ERROR, __VA_ARGS__)

#define YOGI_LOG_WARNING(...) \
    YOGI_LOG(WARNING, __VA_ARGS__)

#define YOGI_LOG_INFO(...) \
    YOGI_LOG(INFO, __VA_ARGS__)

#define YOGI_LOG_DEBUG(...) \
    YOGI_LOG(DEBUG, __VA_ARGS__)

#define YOGI_LOG_TRACE(...) \
    YOGI_LOG(TRACE, __VA_ARGS__)

#define YOGI_LOG(severity, ...)                                           \
    _YOGI_LOG_EXPAND(                                                     \
        _YOGI_LOG_IMPL(severity,                                          \
            _YOGI_LOG_SELECT(_YOGI_LOG_LOGGER, __VA_ARGS__)(__VA_ARGS__), \
            _YOGI_LOG_SELECT(_YOGI_LOG_STREAM, __VA_ARGS__)(__VA_ARGS__)  \
        )                                                                 \
    )

#define _YOGI_LOG_EXPAND(x) \
    x

#define _YOGI_LOG_CAT(a, b) \
   _YOGI_LOG_CAT_(a, b)

#define _YOGI_LOG_CAT_(a, b) \
   a ## b

#define _YOGI_LOG_SELECT(macro, ...) \
    _YOGI_LOG_CAT(macro, _YOGI_LOG_SELECT_(__VA_ARGS__, _CUSTOM_LOGGER, _APP_LOGGER, _INVALID))

#define _YOGI_LOG_SELECT_(_0, _1, suffix, ...) \
    suffix

#define _YOGI_LOG_LOGGER_CUSTOM_LOGGER(logger, stream) \
    logger

#define _YOGI_LOG_LOGGER_APP_LOGGER(stream) \
    yogi::Logger::app_logger()

#define _YOGI_LOG_STREAM_CUSTOM_LOGGER(logger, stream) \
    stream

#define _YOGI_LOG_STREAM_APP_LOGGER(stream) \
    stream

#define _YOGI_LOG_IMPL(severity, logger, stream)                                         \
    {{                                                                                   \
    if (yogi::verbosity::severity <= (logger).max_effective_verbosity()) {               \
        std::stringstream ss;                                                            \
        ss << stream;                                                                    \
        (logger).log(yogi::verbosity::severity, __FILE__, __LINE__, __func__, ss.str()); \
    }                                                                                    \
    }}


namespace yogi {

class Logger
{
    struct VerbosityInfo {
        std::atomic<verbosity> stdoutVerbosity{verbosity::TRACE};
        std::atomic<verbosity> yogiVerbosity{verbosity::TRACE};

        VerbosityInfo();
        VerbosityInfo(const VerbosityInfo& other);
    };

private:
    static std::atomic<unsigned>                          ms_lastSimplifiedThreadId;
    static thread_local std::string                       ms_myThreadName;
    static std::atomic<bool>                              ms_colourisedStdout;
    static VerbosityInfo                                  ms_maxVerbosities;
    static std::unordered_map<std::string, VerbosityInfo> ms_verbosities;
    static std::mutex                                     ms_verbositiesMutex;
    static Logger                                         ms_appLogger;
    static Logger                                         ms_yogiLogger;

    const std::string* m_component;
    VerbosityInfo*     m_verbosities;

public:
    static const std::string& thread_name();
    static void set_thread_name(const std::string& name, bool appendSuffix = false);
    static bool colourised_stdout();
    static void set_colourised_stdout(bool colourised);
    static verbosity max_stdout_verbosity();
    static void set_max_stdout_verbosity(verbosity verb);
    static verbosity max_yogi_verbosity();
    static void set_max_yogi_verbosity(verbosity verb);

    static Logger& app_logger()
    {
        return ms_appLogger;
    }

    static Logger& yogi_logger()
    {
        return ms_yogiLogger;
    }

    static std::vector<Logger> get_all_loggers();

    Logger();
    Logger(const std::string& component);

    const std::string& component() const
    {
        return *m_component;
    }

    verbosity stdout_verbosity() const;
    void set_stdout_verbosity(verbosity verb);
    verbosity yogi_verbosity() const;
    void set_yogi_verbosity(verbosity verb);
    verbosity effective_stdout_verbosity() const;
    verbosity effective_yogi_verbosity() const;
    verbosity max_effective_verbosity() const;

    void log(verbosity severity, const char* file, int line, const char* func, std::string&& message);
};

} // namespace yogi

#endif // YOGI_LOGGING_HPP
