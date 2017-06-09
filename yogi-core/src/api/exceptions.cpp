#include "ExceptionT.hpp"
#include "../yogi_core.h"

#define YOGI_EXCEPTION(error_code_macro, description)                         \
    static auto exception_##error_code_macro =                                 \
        ExceptionT<error_code_macro>::register_exception();                    \
    template<>                                                                 \
    const char* ExceptionT<error_code_macro>::what() const throw()             \
    {                                                                          \
        return description;                                                    \
    }


namespace yogi {
namespace api {

YOGI_EXCEPTION( YOGI_OK,
    "Success");

YOGI_EXCEPTION( YOGI_ERR_UNKNOWN,
    "Unknown internal error occurred");

YOGI_EXCEPTION( YOGI_ERR_INVALID_HANDLE,
    "Invalid object handle");

YOGI_EXCEPTION( YOGI_ERR_WRONG_OBJECT_TYPE,
    "Object is of the wrong type");

YOGI_EXCEPTION( YOGI_ERR_OBJECT_STILL_USED,
    "Object is still used");

YOGI_EXCEPTION( YOGI_ERR_BAD_ALLOCATION,
    "Insufficient memory to complete the operation");

YOGI_EXCEPTION( YOGI_ERR_INVALID_PARAM,
    "Invalid parameter");

YOGI_EXCEPTION( YOGI_ERR_ALREADY_CONNECTED,
    "Already connected");

YOGI_EXCEPTION( YOGI_ERR_AMBIGUOUS_IDENTIFIER,
    "There is already an object with the same identifier");

YOGI_EXCEPTION( YOGI_ERR_ALREADY_INITIALISED,
    "The library has already been initialised");

YOGI_EXCEPTION( YOGI_ERR_NOT_INITIALISED,
    "The library has not been initialised");

YOGI_EXCEPTION( YOGI_ERR_CANNOT_CREATE_LOG_FILE,
    "Could not create the log file");

YOGI_EXCEPTION( YOGI_ERR_CANCELED,
    "The operation has been canceled");

YOGI_EXCEPTION( YOGI_ERR_ASYNC_OPERATION_RUNNING,
    "An asynchronous operation has already been started");

YOGI_EXCEPTION( YOGI_ERR_BUFFER_TOO_SMALL,
    "The provided buffer is not big enough");

YOGI_EXCEPTION( YOGI_ERR_NOT_BOUND,
    "No remote terminals are currently bound to the local terminal");

YOGI_EXCEPTION( YOGI_ERR_INVALID_ID,
    "Invalid ID");

YOGI_EXCEPTION( YOGI_ERR_IDENTIFICATION_TOO_LARGE,
    "Received identification data is too large");

YOGI_EXCEPTION( YOGI_ERR_INVALID_IP_ADDRESS,
    "Invalid IP address");

YOGI_EXCEPTION( YOGI_ERR_INVALID_PORT_NUMBER,
    "Invalid port number");

YOGI_EXCEPTION( YOGI_ERR_CANNOT_OPEN_SOCKET,
    "Could not open socket");

YOGI_EXCEPTION( YOGI_ERR_CANNOT_BIND_SOCKET,
    "Could not bind socket");

YOGI_EXCEPTION( YOGI_ERR_CANNOT_LISTEN_ON_SOCKET,
    "Could not listen on socket");

YOGI_EXCEPTION( YOGI_ERR_SOCKET_BROKEN,
    "Could not read from or write to socket");

YOGI_EXCEPTION( YOGI_ERR_INVALID_MAGIC_PREFIX,
    "Invalid magic prefix received");

YOGI_EXCEPTION( YOGI_ERR_INCOMPATIBLE_VERSION,
    "Incompatible version received");

YOGI_EXCEPTION( YOGI_ERR_ACCEPT_FAILED,
    "Failed to accept incoming connection");

YOGI_EXCEPTION( YOGI_ERR_TIMEOUT,
    "The operation timed out");

YOGI_EXCEPTION( YOGI_ERR_ADDRESS_IN_USE,
    "Address is already in use");

YOGI_EXCEPTION( YOGI_ERR_RESOLVE_FAILED,
    "Could not resolve IP address");

YOGI_EXCEPTION( YOGI_ERR_CONNECTION_REFUSED,
    "Connection refused");

YOGI_EXCEPTION( YOGI_ERR_HOST_UNREACHABLE,
    "Host is unreachable");

YOGI_EXCEPTION( YOGI_ERR_NETWORK_DOWN,
    "Network is down");

YOGI_EXCEPTION( YOGI_ERR_CONNECT_FAILED,
    "Connect failed");

YOGI_EXCEPTION( YOGI_ERR_NOT_READY,
    "Operation is not ready to be performed yet");

YOGI_EXCEPTION( YOGI_ERR_ALREADY_ASSIGNED,
    "The connection has already been assigned");

YOGI_EXCEPTION( YOGI_ERR_CONNECTION_DEAD,
    "The connection is dead");

YOGI_EXCEPTION( YOGI_ERR_CONNECTION_CLOSED,
    "The connection has been closed gracefully by the remote host");

YOGI_EXCEPTION( YOGI_ERR_UNINITIALIZED,
    "Not yet initialized");


} // namespace api
} // namespace yogi
