#include "timestamp.hpp"

#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>


namespace yogi {

Timestamp::Timestamp(clock_type::time_point timePoint)
: m_timePoint(timePoint)
{
}

Timestamp Timestamp::now()
{
    return Timestamp(clock_type::now());
}

std::time_t Timestamp::to_time_t() const
{
    return clock_type::to_time_t(m_timePoint);
}

std::tm Timestamp::to_tm() const
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    auto time = to_time_t();
    auto tm = *std::gmtime(&time);

    return tm;
}

std::chrono::nanoseconds Timestamp::time_since_epoch() const
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(m_timePoint.time_since_epoch());
}

unsigned Timestamp::milliseconds() const
{
    return (time_since_epoch().count() / 1000000) % 1000;
}

unsigned Timestamp::microseconds() const
{
    return (time_since_epoch().count() / 1000) % 1000;
}

unsigned Timestamp::nanoseconds() const
{
    return time_since_epoch().count() % 1000;
}

std::string Timestamp::to_string() const
{
    return to_string(MILLISECONDS);
}

std::string Timestamp::to_string(Precision precision) const
{
    auto tm = to_tm();

    std::stringstream ss;
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y %T", &tm);
    ss << buffer;

    if (precision >= MILLISECONDS) {
        ss << '.' << std::setw(3) << std::setfill('0') << milliseconds();
    }

    if (precision >= MICROSECONDS) {
        ss << '.' << std::setw(3) << std::setfill('0') << microseconds();
    }

    if (precision >= NANOSECONDS) {
        ss << '.' << std::setw(3) << std::setfill('0') << nanoseconds();
    }

    return ss.str();
}

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Timestamp& timestamp)
{
    return os << timestamp.to_string();
}
