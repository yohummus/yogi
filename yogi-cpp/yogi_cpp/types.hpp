#ifndef YOGICPP_TYPES_HPP
#define YOGICPP_TYPES_HPP

#include "signature.hpp"

#include <yogi_core.h>

#include <iostream>
#include <string>


namespace yogi {

enum {
    MAX_MESSAGE_SIZE = 64 * 1024
};

typedef bool cached_flag;
typedef bool operational_flag;
typedef int  raw_operation_id;

enum class verbosity {
    TRACE                    = YOGI_VB_TRACE,
    DEBUG                    = YOGI_VB_DEBUG,
    INFO                     = YOGI_VB_INFO,
    WARNING                  = YOGI_VB_WARNING,
    ERROR                    = YOGI_VB_ERROR,
    FATAL                    = YOGI_VB_FATAL
};

enum control_flow {
    CONTINUE                 = YOGI_DO_CONTINUE,
    STOP                     = YOGI_DO_STOP
};

enum binding_state {
    RELEASED                 = YOGI_BD_RELEASED,
    ESTABLISHED              = YOGI_BD_ESTABLISHED
};

enum subscription_state {
    UNSUBSCRIBED             = YOGI_SB_UNSUBSCRIBED,
    SUBSCRIBED               = YOGI_SB_SUBSCRIBED
};

enum gather_flags {
    NO_FLAGS                 = YOGI_SG_NOFLAGS,
    FINISHED                 = YOGI_SG_FINISHED,
    IGNORED                  = YOGI_SG_IGNORED,
    DEAF                     = YOGI_SG_DEAF,
    BINDING_DESTROYED        = YOGI_SG_BINDINGDESTROYED,
    CONNECTION_LOST          = YOGI_SG_CONNECTIONLOST
};

enum terminal_type {
    DEAF_MUTE                = YOGI_TM_DEAFMUTE,
    PUBLISH_SUBSCRIBE        = YOGI_TM_PUBLISHSUBSCRIBE,
    SCATTER_GATHER           = YOGI_TM_SCATTERGATHER,
    CACHED_PUBLISH_SUBSCRIBE = YOGI_TM_CACHEDPUBLISHSUBSCRIBE,
    PRODUCER                 = YOGI_TM_PRODUCER,
    CONSUMER                 = YOGI_TM_CONSUMER,
    CACHED_PRODUCER          = YOGI_TM_CACHEDPRODUCER,
    CACHED_CONSUMER          = YOGI_TM_CACHEDCONSUMER,
    MASTER                   = YOGI_TM_MASTER,
    SLAVE                    = YOGI_TM_SLAVE,
    CACHED_MASTER            = YOGI_TM_CACHEDMASTER,
    CACHED_SLAVE             = YOGI_TM_CACHEDSLAVE,
    SERVICE                  = YOGI_TM_SERVICE,
    CLIENT                   = YOGI_TM_CLIENT
};

enum change_type {
    REMOVED,
    ADDED
};

struct terminal_info {
    terminal_type type;
    Signature     signature;
    std::string   name;
};

namespace {

gather_flags operator& (gather_flags a, gather_flags b)
{
    return static_cast<gather_flags>(static_cast<int>(a) & static_cast<int>(b));
}

gather_flags operator| (gather_flags a, gather_flags b)
{
    return static_cast<gather_flags>(static_cast<int>(a) | static_cast<int>(b));
}

gather_flags operator~ (gather_flags a)
{
    return static_cast<gather_flags>(~static_cast<int>(a));
}

gather_flags& operator|= (gather_flags& a, gather_flags b)
{
    return a = a | b;
}

gather_flags& operator&= (gather_flags& a, gather_flags b)
{
    return a = a & b;
}

} // anonymous namespace
} // namespace yogi

std::ostream& operator<< (std::ostream& os, yogi::verbosity verb);
std::ostream& operator<< (std::ostream& os, yogi::binding_state state);
std::ostream& operator<< (std::ostream& os, yogi::gather_flags flags);
std::ostream& operator<< (std::ostream& os, yogi::terminal_type type);

#endif // YOGICPP_TYPES_HPP
