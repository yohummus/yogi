#ifndef YOGI_CONFIG_H
#define YOGI_CONFIG_H

// Include the main DLL header file for the error codes
#ifndef YOGI_API
#	ifdef _MSC_VER
#	   define YOGI_API __declspec(dllexport)
#	else
#	   define YOGI_API __attribute__((visibility("default")))
#	endif
#endif

#include "yogi_core.h"

// Boost configuration
#define BOOST_ASIO_HAS_MOVE

// MSVC-specific
#if defined(_MSC_VER)
#   define _WIN32_WINNT _WIN32_WINNT_VISTA
#   define _SCL_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS
#   define _WINSOCK_DEPRECATED_NO_WARNINGS
#   pragma warning (disable: 4250 4503)
#endif

// Constants, limits and defaults
#define YOGI_VERSION                            "0.0.2-alpha"
#define YOGI_MAX_SCHEDULER_THREAD_POOL_SIZE     1000
#define YOGI_DEFAULT_SCHEDULER_THREAD_POOL_SIZE 1
#define YOGI_TCP_ACCEPTOR_BACKLOG               5
#define YOGI_MAX_TCP_IDENTIFICATION_SIZE        16 * 1024
#define YOGI_VERSION_INFO_SIZE                  20
#define YOGI_DEFAULT_TCP_PORT                   41772
#define YOGI_RING_BUFFER_SIZE                   64 * 1024 - 1
#define YOGI_CACHELINE_SIZE                     64

// Debug & development macros
#ifndef NDEBUG
#   include <boost/log/trivial.hpp>
#   define YOGI_ASSERT(x)                                                  \
    {{                                                                     \
        if (!(x)) {                                                        \
            BOOST_LOG_TRIVIAL(fatal) << "ASSERTION " << __FILE__ << ":"    \
                                     << std::dec << __LINE__ << " FAILED"; \
            *(char*)1 = 0;                                                 \
        }                                                                  \
    }}
#   define YOGI_NEVER_REACHED { YOGI_ASSERT(false); }
#   define YOGI_TODO          { YOGI_ASSERT(false); }
#else
#   define YOGI_ASSERT(x)
#   define YOGI_NEVER_REACHED
#   define YOGI_TODO
#endif

#endif // YOGI_CONFIG_H
