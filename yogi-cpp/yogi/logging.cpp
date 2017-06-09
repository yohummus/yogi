#include "logging.hpp"
#include "timestamp.hpp"
#include "process.hpp"
#include "internal/console.hpp"

#include <thread>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <atomic>
using namespace std::string_literals;

#ifdef _WIN32
#   include <Windows.h>
#   undef ERROR
#   undef min
#   undef max
#else
#   include <pthread.h>
#endif


namespace yogi {
namespace internal {
namespace {

void set_colour(verbosity verb)
{
    if (Logger::colourised_stdout()) {
        switch (verb) {
        case verbosity::FATAL:
            internal::set_console_colour(internal::foreground_colour::WHITE);
            internal::set_console_colour(internal::background_colour::RED);
            break;

        case verbosity::ERROR:
            internal::set_console_colour(internal::foreground_colour::LIGHT_RED);
            break;

        case verbosity::WARNING:
            internal::set_console_colour(internal::foreground_colour::YELLOW);
            break;

        case verbosity::INFO:
            internal::set_console_colour(internal::foreground_colour::WHITE);
            break;

        case verbosity::DEBUG:
            internal::set_console_colour(internal::foreground_colour::LIGHT_GREEN);
            break;

        case verbosity::TRACE:
            internal::set_console_colour(internal::foreground_colour::BROWN);
            break;
        }
    }
}

void reset_colours()
{
    if (Logger::colourised_stdout()) {
        internal::reset_console_colours();
    }
}

const char* verbosity_to_tag(verbosity verb)
{
    switch (verb) {
    case verbosity::FATAL:   return "FAT";
    case verbosity::ERROR:   return "ERR";
    case verbosity::WARNING: return "WRN";
    case verbosity::INFO:    return "IFO";
    case verbosity::DEBUG:   return "DBG";
    case verbosity::TRACE:   return "TRC";
    default:                 return nullptr;
    }
}

const char* make_file_name(const char* file)
{
    for (const char* ch = file; *ch; ++ch) {
        if (*ch == '/' || *ch == '\\') {
            file = ch + 1;
        }
    }

    return file;
}

} // anonymous namespace
} // namespace internal

std::atomic<unsigned> Logger::ms_lastSimplifiedThreadId(0);
thread_local std::string Logger::ms_myThreadName;

Logger::VerbosityInfo::VerbosityInfo()
{
}

Logger::VerbosityInfo::VerbosityInfo(const VerbosityInfo& other)
: stdoutVerbosity(other.stdoutVerbosity.load())
, yogiVerbosity (other.yogiVerbosity.load())
{
}

std::atomic<bool>                                      Logger::ms_colourisedStdout(false);
Logger::VerbosityInfo                                  Logger::ms_maxVerbosities;
std::unordered_map<std::string, Logger::VerbosityInfo> Logger::ms_verbosities;
std::mutex                                             Logger::ms_verbositiesMutex;
Logger                                                 Logger::ms_appLogger("App");
Logger                                                 Logger::ms_yogiLogger("Yogi");

const std::string& Logger::thread_name()
{
    if (ms_myThreadName.empty()) {
        ms_myThreadName = "T"s + std::to_string(++ms_lastSimplifiedThreadId);

#ifdef _WIN32
        auto osId = static_cast<unsigned>(GetCurrentThreadId());
#else
        auto osId = static_cast<unsigned>(pthread_self());
#endif

        YOGI_LOG(DEBUG, Logger::yogi_logger(), "Assigned simplified thread ID for thread "
            << osId << " (0x" << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << osId << ")");
    }

    return ms_myThreadName;
}

void Logger::set_thread_name(const std::string& name, bool appendSuffix)
{
    ms_myThreadName = name;
    if (appendSuffix) {
        ms_myThreadName += std::to_string(++ms_lastSimplifiedThreadId);
    }
}

bool Logger::colourised_stdout()
{
    return ms_colourisedStdout;
}

void Logger::set_colourised_stdout(bool colourised)
{
    ms_colourisedStdout = colourised;
}

verbosity Logger::max_stdout_verbosity()
{
    return ms_maxVerbosities.stdoutVerbosity;
}

void Logger::set_max_stdout_verbosity(verbosity verb)
{
    ms_maxVerbosities.stdoutVerbosity = verb;
    ProcessInterface::_on_max_log_verbosity_set(verb, true);
}

verbosity Logger::max_yogi_verbosity()
{
    return ms_maxVerbosities.yogiVerbosity;
}

void Logger::set_max_yogi_verbosity(verbosity verb)
{
    ms_maxVerbosities.yogiVerbosity = verb;
    ProcessInterface::_on_max_log_verbosity_set(verb, false);
}

std::vector<Logger> Logger::get_all_loggers()
{
    std::unordered_map<std::string, VerbosityInfo> verbosities;
    {{
    std::lock_guard<std::mutex> lock(ms_verbositiesMutex);
    verbosities = ms_verbosities;
    }}

    std::vector<Logger> v;
    for (auto& entry : verbosities) {
        v.push_back(entry.first);
    }

    return v;
}

Logger::Logger()
: Logger(std::string())
{
}

Logger::Logger(const std::string& component)
{
    std::lock_guard<std::mutex> lock(ms_verbositiesMutex);
    auto res = ms_verbosities.insert(std::pair<std::string, VerbosityInfo>(component.empty() ? "App"s : component, VerbosityInfo{}));
    m_component   = &res.first->first;
    m_verbosities = &res.first->second;

    if (res.second) {
        ProcessInterface::_on_new_logger_added(this);
    }
}

verbosity Logger::stdout_verbosity() const
{
    return m_verbosities->stdoutVerbosity;
}

void Logger::set_stdout_verbosity(verbosity verb)
{
    m_verbosities->stdoutVerbosity = verb;
    ProcessInterface::_on_log_verbosity_set(*m_component, verb, true);
}

verbosity Logger::yogi_verbosity() const
{
    return m_verbosities->yogiVerbosity;
}

void Logger::set_yogi_verbosity(verbosity verb)
{
    m_verbosities->yogiVerbosity = verb;
    ProcessInterface::_on_log_verbosity_set(*m_component, verb, false);
}

verbosity Logger::effective_stdout_verbosity() const
{
    verbosity max = ms_maxVerbosities.stdoutVerbosity;
    verbosity set = m_verbosities->stdoutVerbosity;
    return std::min(max, set);
}

verbosity Logger::effective_yogi_verbosity() const
{
    verbosity max = ms_maxVerbosities.yogiVerbosity;
    verbosity set = m_verbosities->yogiVerbosity;
    return max < set ? max : set;
}

verbosity Logger::max_effective_verbosity() const
{
    return std::max(effective_stdout_verbosity(), effective_yogi_verbosity());
}

void Logger::log(verbosity severity, const char* file, int line, const char* func, std::string&& message)
{
    auto threadName = thread_name();
    auto timestamp = Timestamp::now();

    if (severity <= effective_stdout_verbosity()) {
        std::stringstream ss1;
        ss1 << timestamp << " [" << threadName << "] ";

        std::stringstream ss2;
        ss2 << internal::verbosity_to_tag(severity) << ' ' << *m_component << ": " << message << '\n';

        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        internal::reset_colours();
        std::cout << ss1.str();
        internal::set_colour(severity);
        std::cout << ss2.str();
        internal::reset_colours();
        std::cout << std::flush;
    }

    if (severity <= effective_yogi_verbosity()) {
        auto fileName = internal::make_file_name(file);
        ProcessInterface::_publish_log_message(severity, fileName, line, func, std::move(message),
            timestamp, threadName, *m_component);
    }
}

} // namespace yogi
