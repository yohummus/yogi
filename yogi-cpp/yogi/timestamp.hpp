#ifndef YOGI_TIMESTAMP_HPP
#define YOGI_TIMESTAMP_HPP

#include <ctime>
#include <chrono>
#include <ostream>


namespace yogi {

class Timestamp
{
public:
    typedef std::chrono::system_clock clock_type;

    enum Precision {
        SECONDS,
        MILLISECONDS,
        MICROSECONDS,
        NANOSECONDS
    };

private:
    const clock_type::time_point m_timePoint;

private:
    Timestamp(clock_type::time_point timePoint);

public:
    static Timestamp now();

    std::time_t to_time_t() const;
    std::tm to_tm() const;
    std::chrono::nanoseconds time_since_epoch() const;
    unsigned milliseconds() const;
    unsigned microseconds() const;
    unsigned nanoseconds() const;

    std::string to_string() const;
    std::string to_string(Precision precision) const;

    bool operator== (const Timestamp& other) const
    {
        return m_timePoint == other.m_timePoint;
    }

    bool operator!= (const Timestamp& other) const
    {
        return m_timePoint != other.m_timePoint;
    }

    bool operator< (const Timestamp& other) const
    {
        return m_timePoint < other.m_timePoint;
    }

    bool operator<= (const Timestamp& other) const
    {
        return m_timePoint <= other.m_timePoint;
    }

    bool operator> (const Timestamp& other) const
    {
        return m_timePoint > other.m_timePoint;
    }

    bool operator>= (const Timestamp& other) const
    {
        return m_timePoint >= other.m_timePoint;
    }
};

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Timestamp& timestamp);

#endif // YOGI_TIMESTAMP_HPP
