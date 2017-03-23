#include "types.hpp"

#include <vector>


std::ostream& operator<< (std::ostream& os, yogi::verbosity verb)
{
    switch (verb) {
    case yogi::verbosity::TRACE:   return os << "TRACE";
    case yogi::verbosity::DEBUG:   return os << "DEBUG";
    case yogi::verbosity::INFO:    return os << "INFO";
    case yogi::verbosity::WARNING: return os << "WARNING";
    case yogi::verbosity::ERROR:   return os << "ERROR";
    case yogi::verbosity::FATAL:   return os << "FATAL";
    default:                           return os;
    }
}

std::ostream& operator<< (std::ostream& os, yogi::binding_state state)
{
    using namespace yogi;

    switch (state) {
    case RELEASED:    return os << "RELEASED";
    case ESTABLISHED: return os << "ESTABLISHED";
    default:          return os << "INVALID";
    }
}

std::ostream& operator<< (std::ostream& os, yogi::gather_flags flags)
{
    using namespace yogi;

    std::vector<const char*> v;
    if (flags == gather_flags::NO_FLAGS) {
        v.push_back("NO_FLAGS");
    }
    else {
        if (flags & gather_flags::FINISHED) {
            v.push_back("FINISHED");
        }
        if (flags & gather_flags::IGNORED) {
            v.push_back("IGNORED");
        }
        if (flags & gather_flags::DEAF) {
            v.push_back("DEAF");
        }
        if (flags & gather_flags::BINDING_DESTROYED) {
            v.push_back("BINDING_DESTROYED");
        }
        if (flags & gather_flags::CONNECTION_LOST) {
            v.push_back("CONNECTION_LOST");
        }
    }

    for (std::size_t i = 0; i < v.size(); ++i) {
        os << v[i];
        if (i < v.size() - 1) {
            os << " | ";
        }
    }

    return os;
}

std::ostream& operator<< (std::ostream& os, yogi::terminal_type type)
{
    using namespace yogi;

    switch (type) {
    case DEAF_MUTE:                return os << "DEAF_MUTE";
    case PUBLISH_SUBSCRIBE:        return os << "PUBLISH_SUBSCRIBE";
    case SCATTER_GATHER:           return os << "SCATTER_GATHER";
    case CACHED_PUBLISH_SUBSCRIBE: return os << "CACHED_PUBLISH_SUBSCRIBE";
    case PRODUCER:                 return os << "PRODUCER";
    case CONSUMER:                 return os << "CONSUMER";
    case CACHED_PRODUCER:          return os << "CACHED_PRODUCER";
    case CACHED_CONSUMER:          return os << "CACHED_CONSUMER";
    case MASTER:                   return os << "MASTER";
    case SLAVE:                    return os << "SLAVE";
    case CACHED_MASTER:            return os << "CACHED_MASTER";
    case CACHED_SLAVE:             return os << "CACHED_SLAVE";
    case SERVICE:                  return os << "SERVICE";
    case CLIENT:                   return os << "CLIENT";
    default:                       return os << "INVALID";
    }
}
