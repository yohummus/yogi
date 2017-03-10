#include "ExceptionT.hpp"
#include "../chirp.h"

#define CHIRP_EXCEPTION(error_code_macro, description)                         \
    static auto exception_##error_code_macro =                                 \
        ExceptionT<error_code_macro>::register_exception();                    \
    template<>                                                                 \
    const char* ExceptionT<error_code_macro>::what() const throw()             \
    {                                                                          \
        return description;                                                    \
    }


namespace chirp {
namespace api {

CHIRP_EXCEPTION( CHIRP_OK,
    "Success");

CHIRP_EXCEPTION( CHIRP_ERR_UNKNOWN,
    "Unknown internal error occurred");

CHIRP_EXCEPTION( CHIRP_ERR_INVALID_HANDLE,
    "Invalid object handle");

CHIRP_EXCEPTION( CHIRP_ERR_WRONG_OBJECT_TYPE,
    "Object is of the wrong type");

CHIRP_EXCEPTION( CHIRP_ERR_OBJECT_STILL_USED,
    "Object is still used");

CHIRP_EXCEPTION( CHIRP_ERR_BAD_ALLOCATION,
    "Insufficient memory to complete the operation");

CHIRP_EXCEPTION( CHIRP_ERR_INVALID_PARAM,
    "Invalid parameter");

CHIRP_EXCEPTION( CHIRP_ERR_ALREADY_CONNECTED,
    "Already connected");

CHIRP_EXCEPTION( CHIRP_ERR_AMBIGUOUS_IDENTIFIER,
    "There is already an object with the same identifier");

CHIRP_EXCEPTION( CHIRP_ERR_ALREADY_INITIALISED,
    "The library has already been initialised");

CHIRP_EXCEPTION( CHIRP_ERR_NOT_INITIALISED,
    "The library has not been initialised");

CHIRP_EXCEPTION( CHIRP_ERR_CANNOT_CREATE_LOG_FILE,
    "Could not create the log file");

CHIRP_EXCEPTION( CHIRP_ERR_CANCELED,
    "The operation has been canceled");

CHIRP_EXCEPTION( CHIRP_ERR_ASYNC_OPERATION_RUNNING,
    "An asynchronous operation has already been started");

CHIRP_EXCEPTION( CHIRP_ERR_BUFFER_TOO_SMALL,
    "The provided buffer is not big enough");

CHIRP_EXCEPTION( CHIRP_ERR_NOT_BOUND,
    "No remote terminals are currently bound to the local terminal");

CHIRP_EXCEPTION( CHIRP_ERR_INVALID_ID,
    "Invalid ID");

CHIRP_EXCEPTION( CHIRP_ERR_IDENTIFICATION_TOO_LARGE,
    "Received identification data is too large");

CHIRP_EXCEPTION( CHIRP_ERR_INVALID_IP_ADDRESS,
    "Invalid IP address");

CHIRP_EXCEPTION( CHIRP_ERR_INVALID_PORT_NUMBER,
    "Invalid port number");

CHIRP_EXCEPTION( CHIRP_ERR_CANNOT_OPEN_SOCKET,
    "Could not open socket");

CHIRP_EXCEPTION( CHIRP_ERR_CANNOT_BIND_SOCKET,
    "Could not bind socket");

CHIRP_EXCEPTION( CHIRP_ERR_CANNOT_LISTEN_ON_SOCKET,
    "Could not listen on socket");

CHIRP_EXCEPTION( CHIRP_ERR_SOCKET_BROKEN,
    "Could not read from or write to socket");

CHIRP_EXCEPTION( CHIRP_ERR_INVALID_MAGIC_PREFIX,
    "Invalid magic prefix received");

CHIRP_EXCEPTION( CHIRP_ERR_INCOMPATIBLE_VERSION,
    "Incompatible version received");

CHIRP_EXCEPTION( CHIRP_ERR_ACCEPT_FAILED,
    "Failed to accept incoming connection");

CHIRP_EXCEPTION( CHIRP_ERR_TIMEOUT,
    "The operation timed out");

CHIRP_EXCEPTION( CHIRP_ERR_ADDRESS_IN_USE,
    "Address is already in use");

CHIRP_EXCEPTION( CHIRP_ERR_RESOLVE_FAILED,
    "Could not resolve IP address");

CHIRP_EXCEPTION( CHIRP_ERR_CONNECTION_REFUSED,
    "Connection refused");

CHIRP_EXCEPTION( CHIRP_ERR_HOST_UNREACHABLE,
    "Host is unreachable");

CHIRP_EXCEPTION( CHIRP_ERR_NETWORK_DOWN,
    "Network is down");

CHIRP_EXCEPTION( CHIRP_ERR_CONNECT_FAILED,
    "Connect failed");

CHIRP_EXCEPTION( CHIRP_ERR_NOT_READY,
    "Operation is not ready to be performed yet");

CHIRP_EXCEPTION( CHIRP_ERR_ALREADY_ASSIGNED,
    "The connection has already been assigned");

CHIRP_EXCEPTION( CHIRP_ERR_CONNECTION_DEAD,
    "The connection is dead");

CHIRP_EXCEPTION( CHIRP_ERR_CONNECTION_CLOSED,
    "The connection has been closed gracefully by the remote host");

CHIRP_EXCEPTION( CHIRP_ERR_UNINITIALIZED,
    "Not yet initialized");


} // namespace api
} // namespace chirp
