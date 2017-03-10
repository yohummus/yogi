#ifndef CHIRP_CONFIG_H
#define CHIRP_CONFIG_H

// Include the main DLL header file for the error codes
#ifndef CHIRP_API
#	ifdef _MSC_VER
#	   define CHIRP_API __declspec(dllexport)
#	else
#	   define CHIRP_API __attribute__((visibility("default")))
#	endif
#endif

#include "chirp.h"

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
#define CHIRP_VERSION                            "0.0.2-alpha"
#define CHIRP_MAX_SCHEDULER_THREAD_POOL_SIZE     1000
#define CHIRP_DEFAULT_SCHEDULER_THREAD_POOL_SIZE 1
#define CHIRP_TCP_ACCEPTOR_BACKLOG               5
#define CHIRP_MAX_TCP_IDENTIFICATION_SIZE        16 * 1024
#define CHIRP_VERSION_INFO_SIZE                  20
#define CHIRP_DEFAULT_TCP_PORT                   41772
#define CHIRP_RING_BUFFER_SIZE                   64 * 1024 - 1
#define CHIRP_CACHELINE_SIZE                     64

// Debug & development macros
#ifndef NDEBUG
#   include <boost/log/trivial.hpp>
#   define CHIRP_ASSERT(x)                                                     \
    {{                                                                         \
        if (!(x)) {                                                            \
            BOOST_LOG_TRIVIAL(fatal) << "ASSERTION " << __FILE__ << ":"        \
                                     << std::dec << __LINE__ << " FAILED";     \
            *(char*)1 = 0;                                                     \
        }                                                                      \
    }}
#   define CHIRP_NEVER_REACHED { CHIRP_ASSERT(false); }
#   define CHIRP_TODO          { CHIRP_ASSERT(false); }
#else
#   define CHIRP_ASSERT(x)
#   define CHIRP_NEVER_REACHED
#   define CHIRP_TODO
#endif

#endif // CHIRP_CONFIG_H
