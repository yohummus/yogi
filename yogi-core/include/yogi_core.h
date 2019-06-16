/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef YOGI_CORE_H
#define YOGI_CORE_H

//! \file
//! \brief Definition the public API of the Yogi Core library.

//! @defgroup VS Version Macros
//!
//! Various macros describing the version of the header file. Note that those
//! macros only denote the version of the header file which does not necessarily
//! have to match the version of the actual library that is loaded at runtime.
//!
//! This header file constitutes the bindings for the C language and therefore,
//! the version macros defined here denote the version of these bindings.
//!
//! @{

#define YOGI_HDR_VERSION "0.0.3-alpha"    ///< Whole version string
#define YOGI_HDR_VERSION_MAJOR 0          ///< Major version number
#define YOGI_HDR_VERSION_MINOR 0          ///< Minor version number
#define YOGI_HDR_VERSION_PATCH 3          ///< Patch version number
#define YOGI_HDR_VERSION_SUFFIX "-alpha"  ///< Version suffix (e.g. "-beta")

//! @}
//!
//! @defgroup CV Constants
//!
//! Immutable values used in the library such as version numbers and default
//! values for function calls. The values of the constants can be obtained via
//! the YOGI_GetConstant() function.
//!
//! The type of each constant is denoted in brackets after its description.
//!
//! @{

//! Whole version string of the library (const char*)
#define YOGI_CONST_VERSION 1

//! Major version number of the library (int)
#define YOGI_CONST_VERSION_MAJOR 2

//! Major version number of the library (int)
#define YOGI_CONST_VERSION_MINOR 3

//! Major version number of the library (int)
#define YOGI_CONST_VERSION_PATCH 4

//! Suffix of the library version (const char*)
#define YOGI_CONST_VERSION_SUFFIX 5

//! Default network interfaces to use for advertising (const char*)
#define YOGI_CONST_DEFAULT_ADV_INTERFACES 6

//! Default multicast addresses to use for advertising (const char*)
#define YOGI_CONST_DEFAULT_ADV_ADDRESS 7

//! Default port to use for advertising via UDP IPv6 multicasts (int)
#define YOGI_CONST_DEFAULT_ADV_PORT 8

//! Default advertising interval in nanoseconds (long long)
#define YOGI_CONST_DEFAULT_ADV_INTERVAL 9

//! Default connection timeout in nanoseconds (long long)
#define YOGI_CONST_DEFAULT_CONNECTION_TIMEOUT 10

//! Default logging verbosity (int)
#define YOGI_CONST_DEFAULT_LOGGER_VERBOSITY 11

//! Default textual format for timestamps in log entries (const char*)
#define YOGI_CONST_DEFAULT_LOG_TIME_FORMAT 12

//! Default textual format for log entries (const char*)
#define YOGI_CONST_DEFAULT_LOG_FORMAT 13

//! Maximum size of the payload in a message (int)
#define YOGI_CONST_MAX_MESSAGE_PAYLOAD_SIZE 14

//! Default textual format for timestamps (const char*)
#define YOGI_CONST_DEFAULT_TIME_FORMAT 15

//! Default string to denote an infinite duration (const char*)
#define YOGI_CONST_DEFAULT_INF_DURATION_STRING 16

//! Default textual format for duration strings (const char*)
#define YOGI_CONST_DEFAULT_DURATION_FORMAT 17

//! Default string to denote an invalid object handle (const char*)
#define YOGI_CONST_DEFAULT_INVALID_HANDLE_STRING 18

//! Default textual format for strings describing an object (const char*)
#define YOGI_CONST_DEFAULT_OBJECT_FORMAT 19

//! Minimum size of a send queue for a remote branch (int)
#define YOGI_CONST_MIN_TX_QUEUE_SIZE 20

//! Maximum size of a send queue for a remote branch (int)
#define YOGI_CONST_MAX_TX_QUEUE_SIZE 21

//! Default size of a send queue for a remote branch (int)
#define YOGI_CONST_DEFAULT_TX_QUEUE_SIZE 22

//! Minimum size of a receive queue for a remote branch (int)
#define YOGI_CONST_MIN_RX_QUEUE_SIZE 23

//! Maximum size of a receive queue for a remote branch (int)
#define YOGI_CONST_MAX_RX_QUEUE_SIZE 24

//! Default size of a receive queue for a remote branch (int)
#define YOGI_CONST_DEFAULT_RX_QUEUE_SIZE 25

//! Default port for the web server to listen on for client connections (int)
#define YOGI_CONST_DEFAULT_WEB_PORT 26

//! Default network interfaces to use for the web server (const char*)
#define YOGI_CONST_DEFAULT_WEB_INTERFACES 27

//! Default timeout for web server connections in nanoseconds (long long)
#define YOGI_CONST_DEFAULT_WEB_TIMEOUT 28

//! Default size of the web server cache in bytes (int)
#define YOGI_CONST_DEFAULT_WEB_CACHE_SIZE 29

//! Maximum size of the web server cache in bytes (int)
#define YOGI_CONST_MAX_WEB_CACHE_SIZE 30

//! Default user name for the administrator account (const char*)
#define YOGI_CONST_DEFAULT_ADMIN_USER 31

//! Default password for the administrator account (const char*)
#define YOGI_CONST_DEFAULT_ADMIN_PASSWORD 32

//! Default private key to use for SSL connections (const char*)
#define YOGI_CONST_DEFAULT_SSL_PRIVATE_KEY 33

//! Default certificate chain to use for SSL connections (const char*)
#define YOGI_CONST_DEFAULT_SSL_CERTIFICATE_CHAIN 34

//! Default DH parameters to use for SSL connections (const char*)
#define YOGI_CONST_DEFAULT_SSL_DH_PARAMS 35

//! @}
//!
//! @defgroup EC Error Codes
//!
//! Yogi error codes indicating failures are always < 0. A human-readable
//! description can be obtained by calling YOGI_GetErrorString().
//!
//! @{

//! Operation completed successfully
#define YOGI_OK 0

//! Unknown internal error occured
#define YOGI_ERR_UNKNOWN -1

//! The object is still being used by another object
#define YOGI_ERR_OBJECT_STILL_USED -2

//! Insufficient memory to complete the operation
#define YOGI_ERR_BAD_ALLOC -3

//! Invalid parameter
#define YOGI_ERR_INVALID_PARAM -4

//! Invalid Handle
#define YOGI_ERR_INVALID_HANDLE -5

//! Object is of the wrong type
#define YOGI_ERR_WRONG_OBJECT_TYPE -6

//! The operation has been canceled
#define YOGI_ERR_CANCELED -7

//! Operation failed because the object is busy
#define YOGI_ERR_BUSY -8

//! The operation timed out
#define YOGI_ERR_TIMEOUT -9

//! The timer has not been started or already expired
#define YOGI_ERR_TIMER_EXPIRED -10

//! The supplied buffer is too small
#define YOGI_ERR_BUFFER_TOO_SMALL -11

//! Could not open a socket
#define YOGI_ERR_OPEN_SOCKET_FAILED -12

//! Could not bind a socket
#define YOGI_ERR_BIND_SOCKET_FAILED -13

//! Could not listen on socket
#define YOGI_ERR_LISTEN_SOCKET_FAILED -14

//! Could not set a socket option
#define YOGI_ERR_SET_SOCKET_OPTION_FAILED -15

//! Invalid regular expression
#define YOGI_ERR_INVALID_REGEX -16

//! Could not read from file
#define YOGI_ERR_READ_FILE_FAILED -17

//! Could not read from or write to socket
#define YOGI_ERR_RW_SOCKET_FAILED -18

//! Could not connect a socket
#define YOGI_ERR_CONNECT_SOCKET_FAILED -19

//! The magic prefix sent when establishing a connection is wrong
#define YOGI_ERR_INVALID_MAGIC_PREFIX -20

//! The Yogi versions are not compatible
#define YOGI_ERR_INCOMPATIBLE_VERSION -21

//! Could not deserialize a message
#define YOGI_ERR_DESERIALIZE_MSG_FAILED -22

//! Could not accept a socket
#define YOGI_ERR_ACCEPT_SOCKET_FAILED -23

//! Attempting to connect branch to itself
#define YOGI_ERR_LOOPBACK_CONNECTION -24

//! The passwords of the local and remote branch don't match
#define YOGI_ERR_PASSWORD_MISMATCH -25

//! The net names of the local and remote branch don't match
#define YOGI_ERR_NET_NAME_MISMATCH -26

//! A branch with the same name is already active
#define YOGI_ERR_DUPLICATE_BRANCH_NAME -27

//! A branch with the same path is already active
#define YOGI_ERR_DUPLICATE_BRANCH_PATH -28

//! Message payload is too large
#define YOGI_ERR_PAYLOAD_TOO_LARGE -29

//! Parsing the command line failed
#define YOGI_ERR_PARSING_CMDLINE_FAILED -30

//! Parsing a JSON string failed
#define YOGI_ERR_PARSING_JSON_FAILED -31

//! Parsing a configuration file failed
#define YOGI_ERR_PARSING_FILE_FAILED -32

//! The configuration is not valid
#define YOGI_ERR_CONFIG_NOT_VALID -33

//! Help/usage text requested
#define YOGI_ERR_HELP_REQUESTED -34

//! Could not write to file
#define YOGI_ERR_WRITE_FILE_FAILED -35

//! One or more configuration variables are undefined or could not be resolved
#define YOGI_ERR_UNDEFINED_VARIABLES -36

//! Support for configuration variables has been disabled
#define YOGI_ERR_NO_VARIABLE_SUPPORT -37

//! A configuration variable has been used in a key
#define YOGI_ERR_VARIABLE_USED_IN_KEY -38

//! Invalid time format
#define YOGI_ERR_INVALID_TIME_FORMAT -39

//! Could not parse time string
#define YOGI_ERR_PARSING_TIME_FAILED -40

//! A send queue for a remote branch is full
#define YOGI_ERR_TX_QUEUE_FULL -41

//! Invalid operation ID
#define YOGI_ERR_INVALID_OPERATION_ID -42

//! Operation is not running
#define YOGI_ERR_OPERATION_NOT_RUNNING -43

//! User-supplied data is not valid MessagePack
#define YOGI_ERR_INVALID_USER_MSGPACK -44

//! Joining UDP multicast group failed
#define YOGI_ERR_JOIN_MULTICAST_GROUP_FAILED -45

//! Enumerating network interfaces failed
#define YOGI_ERR_ENUMERATE_NETWORK_INTERFACES_FAILED -46

//! The section could not be found in the configuration
#define YOGI_ERR_CONFIGURATION_SECTION_NOT_FOUND -47

//! Validating the configuration failed
#define YOGI_ERR_CONFIGURATION_VALIDATION_FAILED -48

//! @}
//!
//! @defgroup VB Log verbosity/severity
//!
//! Verbosity/severity levels for logging.
//!
//! @{

#define YOGI_VB_NONE -1    ///< Used to disable logging
#define YOGI_VB_FATAL 0    ///< Errors that require a process restart
#define YOGI_VB_ERROR 1    ///< Errors that the system can recover from
#define YOGI_VB_WARNING 2  ///< Warnings
#define YOGI_VB_INFO 3     ///< General information about the system state
#define YOGI_VB_DEBUG 4    ///< Information for debugging
#define YOGI_VB_TRACE 5    ///< Detailed debugging

//! @}
//!
//! @defgroup BC Boolean Constants
//!
//! Definitions for true and false.
//!
//! @{

#define YOGI_FALSE 0  ///< Represents a logical false
#define YOGI_TRUE 1   ///< Represents a logical true

//! @}
//!
//! @defgroup ST Stream Constants
//!
//! Constants for differentiating between stdout and stderr.
//!
//! @{

#define YOGI_ST_STDOUT 0  ///< Standard output
#define YOGI_ST_STDERR 1  ///< Standard error output

//! @}
//!
//! @defgroup SIG Signals
//!
//! Definitions of various signals.
//!
//! Signals in Yogi are intended to be used similar to POSIX signals. They have
//! to be raised explicitly by the user (e.g. when receiving an actual POSIX
//! signal like SIGINT) via YOGI_RaiseSignal(). A signal will be received by
//! all signal sets containing that signal.
//!
//! @{

#define YOGI_SIG_NONE 0          ///< No signal
#define YOGI_SIG_INT (1 << 0)    ///< Interrupt (e.g. by pressing STRG + C)
#define YOGI_SIG_TERM (1 << 1)   ///< Termination request
#define YOGI_SIG_USR1 (1 << 24)  ///< User-defined signal 1
#define YOGI_SIG_USR2 (1 << 25)  ///< User-defined signal 2
#define YOGI_SIG_USR3 (1 << 26)  ///< User-defined signal 3
#define YOGI_SIG_USR4 (1 << 27)  ///< User-defined signal 4
#define YOGI_SIG_USR5 (1 << 28)  ///< User-defined signal 5
#define YOGI_SIG_USR6 (1 << 29)  ///< User-defined signal 6
#define YOGI_SIG_USR7 (1 << 30)  ///< User-defined signal 7
#define YOGI_SIG_USR8 (1 << 31)  ///< User-defined signal 8

//! All signals
#define YOGI_SIG_ALL                                               \
  (YOGI_SIG_INT | YOGI_SIG_TERM | YOGI_SIG_USR1 | YOGI_SIG_USR2 |  \
   YOGI_SIG_USR3 | YOGI_SIG_USR4 | YOGI_SIG_USR5 | YOGI_SIG_USR6 | \
   YOGI_SIG_USR7 | YOGI_SIG_USR8)

//! @}
//!
//! @defgroup CFG Configuration Flags
//!
//! Flags used to change a configuration object's behaviour.
//!
//! @{

//! No flags
#define YOGI_CFG_NONE 0

//! Disables support for variables in the configuration
//!
//! Variables are used to define common values (or parts of values) in a
//! pre-defined  section. They can then be used in multiple other parts of the
//! configuration.
//!
//! \note
//!   Variables can *not* be used in keys.
//!
//! Variables are defined in the *variables* section of the configuration:
//!
//! \code
//!   {
//!     "variables": {
//!       "ROOT": "/usr/share/my-app",
//!       "DATA_DIR": "${ROOT}/data"
//!       "DURATION": 30,
//!       "MAX_DURATION": "${DURATION}",
//!   ...
//! \endcode
//!
//! These variables can then be used anywhere in the configuration, even in the
//! *variables* section itself as shown above.
//!
//! \note
//!   Even if the value of a variable is not a string, the placeholder *${name}*
//!   always has to be a string to conform to the JSON format. When resolving a
//!   placeholder for a non-string variable, the type of the target value will
//!   be changed accordingly if and only if the placeholder is surrounded by
//!   quotation marks as shown for the *MAX_DURATION* variable above. Othwise,
//!   the target value will remain a string and the placeholder will be replaced
//!   with the stringified value of the variable.
#define YOGI_CFG_DISABLE_VARIABLES (1 << 0)

//! Makes configuration options given directly on the command line overridable
//!
//! By default, configuration options given direclty on the command line are
//! immutable, i.e. they will never be updated from another source.
#define YOGI_CFG_MUTABLE_CMD_LINE (1 << 1)

//! @}
//!
//! @defgroup CLO Command Line Options
//!
//! Flags used to adjust how command line options are parsed.
//!
//! @{

//! No options
#define YOGI_CLO_NONE 0

//! Include the \c --log... switches for configuring file logging
#define YOGI_CLO_LOGGING (1 << 0)

//! Include the \c --name switch for setting the branch name
#define YOGI_CLO_BRANCH_NAME (1 << 1)

//! Include the \c --description switch for setting the branch description
#define YOGI_CLO_BRANCH_DESCRIPTION (1 << 2)

//! Include the \c --network switch for setting the network name
#define YOGI_CLO_BRANCH_NETWORK (1 << 3)

//! Include the \c --password switch for setting the network password
#define YOGI_CLO_BRANCH_PASSWORD (1 << 4)

//! Include the \c --path switch for setting the branch path
#define YOGI_CLO_BRANCH_PATH (1 << 5)

//! Include the \c --adv-interfaces switch for setting the branch advertising
//! interfaces
#define YOGI_CLO_BRANCH_ADV_IFS (1 << 6)

//! Include the \c --adv-addr switch for setting the branch advertising address
#define YOGI_CLO_BRANCH_ADV_ADDR (1 << 7)

//! Include the \c --adv-port switch for setting the branch advertising port
#define YOGI_CLO_BRANCH_ADV_PORT (1 << 8)

//! Include the \c --adv-int switch for setting the branch advertising interval
#define YOGI_CLO_BRANCH_ADV_INT (1 << 9)

//! Include the \c --timeout switch for setting the branch timeout
#define YOGI_CLO_BRANCH_TIMEOUT (1 << 10)

//! Include the \c --ghost_mode switch for enabling ghost mode for the branch
#define YOGI_CLO_BRANCH_GHOST_MODE (1 << 11)

//! Parse configuration files given on the command line
//!
//! The files will be parsed from left to right, i.e. if the same value is set
//! in two supplied configuration files, then the value from the rightmost file
//! will be used. However, values given directly on the command line, i.e. not
//! through files, have higher priority.
#define YOGI_CLO_FILES (1 << 12)

//! Same as #YOGI_CLO_FILES but at least one configuration must be given
#define YOGI_CLO_FILES_REQUIRED (1 << 13)

//! Include the \c --override switch for overriding arbitrary configuration
//! sections
//!
//! This is useful for supplying arbitrary parameters on the command line
//! without having to store them in a file.
//!
//! \note
//!   Parameters supplied in this way override the same parameters in any
//!   given configuration file. If the same parameter is set directly on the
//!   command line multiple times, then the rightmost value is used.
#define YOGI_CLO_OVERRIDES (1 << 14)

//! Include the \c --var switch for setting variables
#define YOGI_CLO_VARIABLES (1 << 15)

//! Combination of all branch flags
#define YOGI_CLO_BRANCH_ALL                                                    \
  (YOGI_CLO_BRANCH_NAME | YOGI_CLO_BRANCH_DESCRIPTION |                        \
   YOGI_CLO_BRANCH_NETWORK | YOGI_CLO_BRANCH_PASSWORD | YOGI_CLO_BRANCH_PATH | \
   YOGI_CLO_BRANCH_ADV_IFS | YOGI_CLO_BRANCH_ADV_ADDR |                        \
   YOGI_CLO_BRANCH_ADV_PORT | YOGI_CLO_BRANCH_ADV_INT |                        \
   YOGI_CLO_BRANCH_TIMEOUT | YOGI_CLO_BRANCH_GHOST_MODE)

//! Combination of all flags
//!
//! This is usually used when using the application object.
#define YOGI_CLO_ALL                                         \
  (YOGI_CLO_LOGGING | YOGI_CLO_BRANCH_ALL | YOGI_CLO_FILES | \
   YOGI_CLO_FILES_REQUIRED | YOGI_CLO_OVERRIDES | YOGI_CLO_VARIABLES)

//! @}
//!
//! @defgroup BEV Branch Events
//!
//! Definitions of various events that can be observed on a branch.
//!
//! @{

//! No event (passed to event handler function if wait operation failed)
#define YOGI_BEV_NONE 0

//! A new branch has been discovered
//!
//! A remote branch is considered to be a _new branch_ if it is neither already
//! connected nor in the process of being connected to. This means if we
//! discover a branch and connect to it but connecting to it fails and the
//! remote branch itself has not connected to us, then the next time an
//! we receive an advertisement message we consider the branch new again.
//!
//! Associated event information:
//!
//! \code
//!   {
//!     "uuid":               "123e4567-e89b-12d3-a456-426655440000",
//!     "tcp_server_address": "fe80::f086:b106:2c1b:c45",
//!     "tcp_server_port":    43384
//!   }
//! \endcode
#define YOGI_BEV_BRANCH_DISCOVERED (1 << 0)

//! Querying a new branch for information finished (successfully or not)
//!
//! Associated event information:
//!
//! \code
//!   {
//!     "uuid":                 "123e4567-e89b-12d3-a456-426655440000",
//!     "name":                 "Pump Safety Logic",
//!     "description":          "Monitors the pump for safety",
//!     "network_name":         "Hardware Control",
//!     "path":                 "/Cooling System/Pump/Safety",
//!     "hostname":             "beaglebone",
//!     "pid":                  3321,
//!     "tcp_server_address":   "fe80::f086:b106:2c1b:c45",
//!     "tcp_server_port":      43384,
//!     "start_time":           "2018-04-23T18:25:43.511Z",
//!     "timeout":              3.0,
//!     "advertising_interval": 1.0,
//!     "ghost_mode":           false
//!   }
//! \endcode
#define YOGI_BEV_BRANCH_QUERIED (1 << 1)

//! Connecting to a branch finished (successfully or not)
//!
//! Associated event information:
//!
//! \code
//!   {
//!     "uuid": "123e4567-e89b-12d3-a456-426655440000"
//!   }
//! \endcode
#define YOGI_BEV_CONNECT_FINISHED (1 << 2)

//! The connection to a branch was lost
//!
//! Associated event information:
//!
//! \code
//!   {
//!     "uuid": "123e4567-e89b-12d3-a456-426655440000"
//!   }
//! \endcode
#define YOGI_BEV_CONNECTION_LOST (1 << 3)

//! All branch events
#define YOGI_BEV_ALL                                      \
  (YOGI_BEV_BRANCH_DISCOVERED | YOGI_BEV_BRANCH_QUERIED | \
   YOGI_BEV_CONNECT_FINISHED | YOGI_BEV_CONNECTION_LOST)

//! @}
//!
//! @defgroup ENC Encoding Types
//!
//! Possible data/payload encoding types.
//!
//! @{

#define YOGI_ENC_JSON 0     ///< Data is encoded as JSON
#define YOGI_ENC_MSGPACK 1  ///< Data is encoded as MessagePack

//! @}
//!
//! @defgroup RLS Provider-Consumer Role Source
//!
//! When creating a terminal or interface, a provider/consumer role has to be
//! assigned. The definitions in this section set where this role information
//! comes from.
//!
//! @{

//! Role is determined by the value of the _role_ string from the provided
//! terminal properties.
//!
//! For example, the following will set the role to _Provider_:
//!
//! \code
//!   {
//!     "role": "Provider",
//!     ...
//!   }
//! \endcode
#define YOGI_RLS_JSON 0

//! Role is determined by the value of the _role_ string from the provided
//! terminal properties but inverted.
//!
//! For example, the following will set the role to _Consumer_:
//!
//! \code
//!   {
//!     "role": "Provider",
//!     ...
//!   }
//! \endcode
#define YOGI_RLS_JSON_INV 1

//! Role is set to _Provider_.
#define YOGI_RLS_PROVIDER 2

//! Role is set to _Consumer_.
#define YOGI_RLS_CONSUMER 3

//! @}
//!
//! @defgroup REQ HTTP request methods
//!
//! Request methods supported by the built-in web server.
//!
//! @{

//! No methods
#define YOGI_MET_NONE 0

//! GET method for retrieving data.
#define YOGI_MET_GET (1 << 0)

//! HEAD method for retrieving the same header as with the GET request but
//! without the response body.
#define YOGI_MET_HEAD (1 << 1)

//! POST method for submitting data.
#define YOGI_MET_POST (1 << 2)

//! PUT method for modifying data.
#define YOGI_MET_PUT (1 << 3)

//! DELETE method for deleting data.
#define YOGI_MET_DELETE (1 << 4)

//! PATCH method for partially modifying data.
#define YOGI_MET_PATCH (1 << 5)

//! @}
//!
//! @defgroup HTTP response status codes
//!
//! Status codes for responses to HTTP requests.
//!
//! @{

#define YOGI_HTTP_200 200  ///< OK
#define YOGI_HTTP_201 201  ///< Created
#define YOGI_HTTP_202 202  ///< Accepted
#define YOGI_HTTP_203 203  ///< Non-Authoritative Information
#define YOGI_HTTP_204 204  ///< No Content
#define YOGI_HTTP_205 205  ///< Reset Content
#define YOGI_HTTP_206 206  ///< Partial Content
#define YOGI_HTTP_300 300  ///< Multiple Choices
#define YOGI_HTTP_301 301  ///< Moved Permanently
#define YOGI_HTTP_302 302  ///< Found
#define YOGI_HTTP_303 303  ///< See Other
#define YOGI_HTTP_304 304  ///< Not Modified
#define YOGI_HTTP_307 307  ///< Temporary Redirect
#define YOGI_HTTP_308 308  ///< Permanent Redirect
#define YOGI_HTTP_400 400  ///< Bad Request
#define YOGI_HTTP_401 401  ///< Unauthorized
#define YOGI_HTTP_403 403  ///< Forbidden
#define YOGI_HTTP_404 404  ///< Not Found
#define YOGI_HTTP_405 405  ///< Method Not Allowed
#define YOGI_HTTP_406 406  ///< Not Acceptable
#define YOGI_HTTP_407 407  ///< Proxy Authentication Required
#define YOGI_HTTP_408 408  ///< Request Timeout
#define YOGI_HTTP_409 409  ///< Conflict
#define YOGI_HTTP_410 410  ///< Gone
#define YOGI_HTTP_411 411  ///< Length Required
#define YOGI_HTTP_412 412  ///< Precondition Failed
#define YOGI_HTTP_413 413  ///< Payload Too Large
#define YOGI_HTTP_414 414  ///< URI Too Long
#define YOGI_HTTP_415 415  ///< Unsupported Media Type
#define YOGI_HTTP_416 416  ///< Range Not Satisfiable
#define YOGI_HTTP_417 417  ///< Expectation Failed
#define YOGI_HTTP_418 418  ///< I'm a teapot
#define YOGI_HTTP_422 422  ///< Unprocessable Entity
#define YOGI_HTTP_425 425  ///< Too Early
#define YOGI_HTTP_426 426  ///< Upgrade Required
#define YOGI_HTTP_428 428  ///< Precondition Required
#define YOGI_HTTP_429 429  ///< Too Many Requests
#define YOGI_HTTP_431 431  ///< Request Header Fields Too Large
#define YOGI_HTTP_451 451  ///< Unavailable For Legal Reasons
#define YOGI_HTTP_500 500  ///< Internal Server Error
#define YOGI_HTTP_501 501  ///< Not Implemented
#define YOGI_HTTP_502 502  ///< Bad Gateway
#define YOGI_HTTP_503 503  ///< Service Unavailable
#define YOGI_HTTP_504 504  ///< Gateway Timeout
#define YOGI_HTTP_505 505  ///< HTTP Version Not Supported
#define YOGI_HTTP_511 511  ///< Network Authentication Required

//! @}
//!
//! @defgroup WPA Web process actions
//!
//! Actions on a web process, originating from a web client.
//!
//! @{

#define YOGI_WPA_START 0     ///< Client requests to start a process
#define YOGI_WPA_INPUT 1     ///< Client sent some input to the process
#define YOGI_WPA_CANCELED 2  ///< Client canceled the process

//! @}
//!
//! @defgroup WPU Web process updates
//!
//! Updates from a web process, originating from the web server.
//!
//! @{

#define YOGI_WPU_STARTED 0          ///< Process has been started
#define YOGI_WPU_FAILED_TO_START 1  ///< Process could not be started
#define YOGI_WPU_FINISHED 2         ///< Process finished gracefully
#define YOGI_WPU_CRASHED 3          ///< Process finished by crashing
#define YOGI_WPU_CANCELED 4         ///< Process finished by cancelation
#define YOGI_WPU_OUTPUT 5           ///< Process created some output

//! @}

#ifndef YOGI_API
#ifdef _MSC_VER
#define YOGI_API __declspec(dllimport)
#else
#define YOGI_API
#endif
#endif

//! @defgroup FN Library Functions
//!
//! Description of the various library functions.
//!
//! @{

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Get the version of the loaded library.
 *
 * The returned string is human-reable and in the same format as the
 * #YOGI_HDR_VERSION macro. Note that those two strings do not necessarily have
 * to match since the final executable can load a library with a version number
 * different from the version number in the used header file.
 *
 * \returns Library version
 */
YOGI_API const char* YOGI_GetVersion();

/*!
 * Checks if the bindings for Yogi used in a program are compatible with the
 * loaded Yogi core library.
 *
 * This function should be used straight after loading the Yogi library in order
 * to ensure that the user code is compatible with the library. All official
 * Yogi bindings (yogi-cpp, yogi-python, etc.) perform this check automatically
 * on startup and terminate the program if the library version is not compatible
 * with the bindings.
 *
 * Bindings are compatible if and only if the major version numbers are
 * identical and the minor version number of the bindings is greater or equal to
 * the minor version number of the Yogi library.
 *
 * In case that the versions are not compatible, the \p err parameter can be
 * used to obtain a message that can be shown to the end user in order to
 * inform them about the version mismatch. If the string to be written to \p err
 * is larger than \p errsize then \p err will be populated as much as possible
 * (and null-terminated).
 *
 * Attention: Even though an error code will be returned if the versions are
 *            incompatible or the supplied version string is invalid, the
 *            returned value should be compared only against 0 (#YOGI_OK) since
 *            error codes may change between versions!
 *
 * \param[in]  bindver Version of the bindings
 * \param[out] err     Pointer to a char array for storing an error description
 *                     (can be set to NULL)
 * \param[in]  errsize Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] if versions are not compatible
 */
YOGI_API int YOGI_CheckBindingsCompatibility(const char* bindver, char* err,
                                             int errsize);

/*!
 * Get the license that Yogi is published under.
 *
 * \note
 *   The returned string contains the complete description of the license
 *   and is therefore very large.
 *
 * \returns License information
 */
YOGI_API const char* YOGI_GetLicense();

/*!
 * Get the license information about the 3rd party libraries used in Yogi.
 *
 * \note
 *   The returned string is very large.
 *
 * \returns License information
 */
YOGI_API const char* YOGI_Get3rdPartyLicenses();

/*!
 * Get a description of an error code.
 *
 * Returns a human-readable string describing the given error code. The returned
 * string is always a valid, null-terminated and human-readable string, even if
 * the supplied error code is not valid.
 *
 * \param[in] err Error code (see \ref EC)
 *
 * \returns Description of the error code
 */
YOGI_API const char* YOGI_GetErrorString(int err);

/*!
 * Get the value of a constant (see \ref CV).
 *
 * Depending on the type of constant, which can either be an integer number or a
 * null-terminated string (see \ref CV), the target of the \p dest parameter
 * will be set accordingly: For integer-type constants, \p dest will be treated
 * as a pointer to an int and its target will be set to the value of the
 * constant; for string-type constants, \p dest will be treated as a pointer to
 * a char* string and its target will be set to the address of the string
 * constant.
 *
 * \param[out] dest     Pointer to where the value will be written to
 * \param[in]  constant The constant to retrieve (see \ref CV)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_GetConstant(void* dest, int constant);

/*!
 * Get the current time.
 *
 * \param[out] timestamp Current time in nanoseconds since 01/01/1970 UTC.
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_GetCurrentTime(long long* timestamp);

/*!
 * Converts a timestamp into a string.
 *
 * The \p timefmt parameter describes the format of the conversion. The
 * following placeholders are supported:
 *  - \c \%Y: Four digit year
 *  - \c \%m: Month name as a decimal 01 to 12
 *  - \c \%d: Day of the month as decimal 01 to 31
 *  - \c \%F: Equivalent to %Y-%m-%d (the ISO 8601 date format)
 *  - \c \%H: The hour as a decimal number using a 24-hour clock (00 to 23)
 *  - \c \%M: The minute as a decimal 00 to 59
 *  - \c \%S: Seconds as a decimal 00 to 59
 *  - \c \%T: Equivalent to %H:%M:%S (the ISO 8601 time format)
 *  - \c \%3: Milliseconds as decimal number 000 to 999
 *  - \c \%6: Microseconds as decimal number 000 to 999
 *  - \c \%9: Nanoseconds as decimal number 000 to 999
 *
 * \note
 *   The placeholder syntax is a modulo sign followed by a single character. Any
 *   additional characters shown above are for Doxygen.
 *
 * If \p timefmt is set to NULL, then the timestamp will be formatted as an
 * ISO-8601 string with up to milliseconds resolution, e.g.
 * "2009-02-11T12:53:09.123Z".
 *
 * \param[in]  timestamp Timestamp in nanoseconds since 01/01/1970 UTC
 * \param[out] str       Pointer to a string for storing the result
 * \param[in]  strsize   Maximum number of bytes to write to \p str
 * \param[in]  timefmt   Format of the time string (set to NULL for default)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_FormatTime(long long timestamp, char* str, int strsize,
                             const char* timefmt);

/*!
 * Converts a string into a timestamp.
 *
 * The \p timefmt parameter describes the format of the conversion. The
 * following placeholders are supported:
 *  - \c \%Y: Four digit year
 *  - \c \%m: Month name as a decimal 01 to 12
 *  - \c \%d: Day of the month as decimal 01 to 31
 *  - \c \%F: Equivalent to %Y-%m-%d (the ISO 8601 date format)
 *  - \c \%H: The hour as a decimal number using a 24-hour clock (00 to 23)
 *  - \c \%M: The minute as a decimal 00 to 59
 *  - \c \%S: Seconds as a decimal 00 to 59
 *  - \c \%T: Equivalent to %H:%M:%S (the ISO 8601 time format)
 *  - \c \%3: Milliseconds as decimal number 000 to 999
 *  - \c \%6: Microseconds as decimal number 000 to 999
 *  - \c \%9: Nanoseconds as decimal number 000 to 999
 *
 * \note
 *   The placeholder syntax is a modulo sign followed by a single character. Any
 *   additional characters shown above are for Doxygen.
 *
 * If \p timefmt is set to NULL, then the timestamp will be parsed in the
 * format "2009-02-11T12:53:09.123Z".
 *
 * \param[out] timestamp Resulting in nanoseconds since 01/01/1970 UTC
 * \param[in]  str       String to parse
 * \param[in]  timefmt   Format of the time string (set to NULL for default)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ParseTime(long long* timestamp, const char* str,
                            const char* timefmt);

/*!
 * Converts a duration into a string.
 *
 * The \p durfmt parameter describes the format of the conversion. The
 * following placeholders are supported:
 *  - \c \%+: Plus sign if duration is positive and minus sign if it is negative
 *  - \c \%-: Minus sign (only) if duration is negative
 *  - \c \%d: Total number of days
 *  - \c \%D: Total number of days if days > 0
 *  - \c \%H: Fractional hours (range 00 to 23)
 *  - \c \%M: Fractional minutes (range 00 to 59)
 *  - \c \%S: Fractional seconds (range 00 to 59)
 *  - \c \%T: Equivalent to %H:%M:%S
 *  - \c \%3: Fractional milliseconds (range 000 to 999)
 *  - \c \%6: Fractional microseconds (range 000 to 999)
 *  - \c \%9: Fractional nanoseconds (range 000 to 999)
 *
 * The \p inffmt parameter describes the format to use for infinite durations.
 * The following placeholders are supported:
 *  - \c \%+: Plus sign if duration is positive and minus sign if it is negative
 *  - \c \%-: Minus sign (only) if duration is negative
 *
 * \note
 *   The placeholder syntax is a modulo sign followed by a single character. Any
 *   additional characters shown above are for Doxygen.
 *
 * If \p durfmt is set to NULL, then the duration will be formatted in the
 * format "-23d 04:19:33.123456789". If \p dur is -1 to indicate an infinite
 * duration, then \p infstr will be copied to \p str. If \p infstr is set to
 * NULL, then the format string "%-inf" will be used.
 *
 * \param[in]  dur     Duration in nanoseconds (-1 for infinity or >= 0)
 * \param[in]  neg     Duration is negative (#YOGI_TRUE or #YOGI_FALSE)
 * \param[out] str     Pointer to a string for storing the result
 * \param[in]  strsize Maximum number of bytes to write to \p str
 * \param[in]  durfmt  Format of the duration string (set to NULL for default)
 * \param[in]  inffmt  Format to use for infinity (set to NULL for default)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_FormatDuration(long long dur, int neg, char* str, int strsize,
                                 const char* durfmt, const char* inffmt);

/*!
 * Creates a string describing an object.
 *
 * The \p objfmt parameter describes the format of the string. The following
 * placeholders are supported:
 *  - \c $T: Type of the object (e.g. Branch)
 *  - \c $x: Handle of the object in lower-case hex notation
 *  - \c $X: Handle of the object in upper-case hex notation
 *
 * \note
 *   The placeholder syntax is a dollar sign followed by a single character. Any
 *   additional characters shown above are for Doxygen.
 *
 * If \p objfmt is set to NULL, then the object will be formatted in the format
 * "Branch [44fdde]" with the hex value in brackets being the object's handle,
 * i.e. the address of the \p obj pointer. If \p obj is NULL then \p nullstr
 * will be copied to \p str. If \p nullstr is set to NULL, then the string
 * "INVALID HANDLE" will be used.
 *
 * \param[in]  object  Handle of the object to print
 * \param[out] str     Pointer to a string for storing the result
 * \param[in]  strsize Maximum number of bytes to write to \p str
 * \param[in]  objfmt  Format of the string (set to NULL for default)
 * \param[in]  nullstr String to use if \p obj is NULL
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_FormatObject(void* object, char* str, int strsize,
                               const char* objfmt, const char* nullstr);

/*!
 * Configures logging to the console.
 *
 * This function supports colourizing the output if the terminal that the
 * process is running in supports it. The color used for a log entry depends on
 * the entry's severity. For example, errors will be printed in red and warnings
 * in yellow.
 *
 * Writing to the console can be disabled by setting \p verbosity to
 * #YOGI_VB_NONE.
 *
 * Each log entry contains the _component_ tag which describes which part of a
 * program issued the log entry. For entries created by the library itself, this
 * parameter is prefixed with the string "Yogi.", followed by the internal
 * component name. For example, the component tag for a branch would be
 * "Yogi.Branch".
 *
 * The \p timefmt parameter describes the textual format of a log entry's
 * timestamp. The following placeholders are supported:
 *  - \c \%Y: Four digit year
 *  - \c \%m: Month name as a decimal 01 to 12
 *  - \c \%d: Day of the month as decimal 01 to 31
 *  - \c \%F: Equivalent to %Y-%m-%d (the ISO 8601 date format)
 *  - \c \%H: The hour as a decimal number using a 24-hour clock (00 to 23)
 *  - \c \%M: The minute as a decimal 00 to 59
 *  - \c \%S: Seconds as a decimal 00 to 59
 *  - \c \%T: Equivalent to %H:%M:%S (the ISO 8601 time format)
 *  - \c \%3: Milliseconds as decimal number 000 to 999
 *  - \c \%6: Microseconds as decimal number 000 to 999
 *  - \c \%9: Nanoseconds as decimal number 000 to 999
 *
 * The \p fmt parameter describes the textual format of the complete log entry
 * as it will appear on the console. The supported placeholders are:
 *  - \c $t: Timestamp, formatted according to \p timefmt
 *  - \c $P: Process ID (PID)
 *  - \c $T: Thread ID
 *  - \c $s: Severity as 3 letter abbreviation (FAT, ERR, WRN, IFO, DBG or TRC)
 *  - \c $m: Log message
 *  - \c $f: Source filename
 *  - \c $l: Source line number
 *  - \c $c: Component tag
 *  - \c $<: Set console color corresponding to severity
 *  - \c $>: Reset the colours (also done after each log entry)
 *  - \c $$: A $ sign
 *
 * \note
 *   The placeholder syntax is a modulo or dollar sign repectively, followed by
 *   a single character. Any additional characters shown above are for Doxygen.
 *
 * \param[in] verbosity Maximum verbosity of messages to log
 * \param[in] stream    The stream to use (#YOGI_ST_STDOUT or #YOGI_ST_STDERR)
 * \param[in] color     Use colors in output (#YOGI_TRUE or #YOGI_FALSE)
 * \param[in] timefmt   Format of the timestamp (set to NULL for default)
 * \param[in] fmt       Format of a log entry (set to NULL for default)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigureConsoleLogging(int verbosity, int stream, int color,
                                          const char* timefmt, const char* fmt);

/*!
 * Configures logging to a user-defined function.
 *
 * This function can be used to get notified whenever the Yogi library itself or
 * the user produces log messages. These messages can then be processed further
 * in user code.
 *
 * Only one callback function can be registered. Calling
 * YOGI_ConfigureHookLogging() again will override the previous function.
 * Setting \p fn to NULL or \p verbosity to #YOGI_VB_NONE will disable the hook.
 *
 * \note
 *   The library will call \p fn from only one thread at a time, i.e. \p fn does
 *   not have to be thread-safe.
 *
 * The parameters passed to \p fn are:
 *  -# __severity__: Severity (verbosity) of the entry (see \ref VB)
 *  -# __timestamp__: Timestamp of the entry in nanoseconds since 01/01/1970 UTC
 *  -# __tid__: ID of the thread that created the entry
 *  -# __file__: Source file name
 *  -# __line__: Source file line number
 *  -# __comp__: Component that created the entry
 *  -# __msg__: Log message
 *  -# __userarg__: Value of \p userarg
 *
 * \attention
 *   The two string arguments *comp* and *msg* of \p fn are valid only while
 *   \p fn is being executed. Do not access those variables at a later time!
 *
 * \param[in] verbosity Maximum verbosity of messages to call \p fn for
 * \param[in] fn        Callback function
 * \param[in] userarg   User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigureHookLogging(
    int verbosity,
    void (*fn)(int severity, long long timestamp, int tid, const char* file,
               int line, const char* comp, const char* msg, void* userarg),
    void* userarg);

/*!
 * Configures logging to a file.
 *
 * This function opens a file to write library-internal and user logging
 * information to. If the file with the given filename already exists then it
 * will be overwritten.
 *
 * Writing to a log file can be disabled by setting \p filename to NULL or
 * \p verbosity to #YOGI_VB_NONE.
 *
 * The \p timefmt and \p fmt parameters describe the textual format for a log
 * entry. The \p filename parameter supports all placeholders that are valid
 * for \p timefmt. See the YOGI_ConfigureConsoleLogging() function for supported
 * placeholders.
 *
 * \note
 *   The color-related placeholders are ignored when writing to log files.
 *
 * The \p genfn parameter can be used to obtain the filename generated by
 * replacing the placeholders in the \p filename parameter. If \p genfnsize
 * is too small, the function returns #YOGI_ERR_BUFFER_TOO_SMALL, however, the
 * log file will still be created and \p genfn will be populated as much as
 * possible.
 *
 * \param[in]  verbosity Maximum verbosity of messages to log to stderr
 * \param[in]  filename  Path to the log file (see description for placeholders)
 * \param[out] genfn     Pointer to a string for storing the generated filename
 *                       (can be set to NULL)
 * \param[in]  genfnsize Maximum number of bytes to write to \p genfn
 * \param[in]  timefmt   Format of the timestamp (set to NULL for default)
 * \param[in]  fmt       Format of a log entry (set to NULL for default)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigureFileLogging(int verbosity, const char* filename,
                                       char* genfn, int genfnsize,
                                       const char* timefmt, const char* fmt);

/*!
 * Creates a logger.
 *
 * A logger is an object used for generating log entries that are tagged with
 * the logger's component tag. A logger's component tag does not have to be
 * unique, i.e. multiple loggers can be created using identical \p component
 * strings.
 *
 * The verbosity of new loggers is #YOGI_VB_INFO by default.
 *
 * \param[out] logger    Pointer to the logger handle
 * \param[in]  component The component tag to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_LoggerCreate(void** logger, const char* component);

/*!
 * Gets the verbosity of a particular logger.
 *
 * The verbosity of a logger acts as a filter. Only messages with a verbosity
 * less than or equal to the given value are being logged.
 *
 * \note
 *   The verbosity of a logger affects only messages logged through that
 *   particular logger, i.e. if two loggers have identical component tags their
 *   verbosity settings are still independent from each other.
 *
 * \param[in]  logger    Logger handle (set to NULL for the App logger)
 * \param[out] verbosity Pointer to where the verbosity level shall be written
 *                       to (see \ref VB)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_LoggerGetVerbosity(void* logger, int* verbosity);

/*!
 * Sets the verbosity of a particular logger.
 *
 * The verbosity of a logger acts as a filter. Only messages with a verbosity
 * less than or equal to the given value are being logged.
 *
 * \note
 *   The verbosity of a logger affects only messages logged through that
 *   particular logger, i.e. if two loggers have identical component tags their
 *   verbosity settings are still independent from each other.
 *
 * \param[in] logger    Logger handle (set to NULL for the App logger)
 * \param[in] verbosity Maximum verbosity entries to be logged (see \ref VB)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_LoggerSetVerbosity(void* logger, int verbosity);

/*!
 * Sets the verbosity of all loggers matching a given component tag.
 *
 * This function finds all loggers whose component tag matches the regular
 * expression given in the \p components parameter and sets their verbosity
 * to \p verbosity.
 *
 * \param[in]  components Regex (ECMAScript) for the component tags to match
 * \param[in]  verbosity  Maximum verbosity entries to be logged (see \ref VB)
 * \param[out] count      Number of matching loggers (can be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_LoggerSetComponentsVerbosity(const char* components,
                                               int verbosity, int* count);

/*!
 * Creates a log entry.
 *
 * The entry can be generated using a specific logger or, by setting \p logger
 * to NULL, the App logger will be used. The App logger is always present and
 * uses the string "App" as the component tag.
 *
 * \param[in] logger   The logger to use (set to NULL for the App logger)
 * \param[in] severity Severity (verbosity) of the entry (see \ref VB)
 * \param[in] file     Source file name (can be set to NULL)
 * \param[in] line     Source file line number (can be set to 0)
 * \param[in] msg      Log message
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_LoggerLog(void* logger, int severity, const char* file,
                            int line, const char* msg);

/*!
 * Creates a configuration.
 *
 * A configuration represents a set of parameters that usually remain constant
 * throughout the runtime of a program. Parameters can come from different
 * sources such as the command line or a file. Configurations are used for
 * other parts of the library such as application objects, however, they are
 * also intended to store user-defined parameters.
 *
 * The \p flags parameter is used to change the behaviour of a configuration
 * object in certain key areas (see \ref CFG).
 *
 * \param[out] config Pointer to the configuration handle
 * \param[in]  flags  See \ref CFG for possible behaviour adjustments
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationCreate(void** config, int flags);

/*!
 * Updates a configuration from command line options.
 *
 * The function populates \p err with:
 * - a description of the error if the function returns one of
 *   #YOGI_ERR_PARSING_CMDLINE_FAILED, #YOGI_ERR_PARSING_FILE_FAILED,
 *   #YOGI_ERR_PARSING_JSON_FAILED or #YOGI_ERR_CONFIG_NOT_VALID; and
 * - the help/usage text if the function returns #YOGI_ERR_HELP_REQUESTED; and
 * - an empty, null-terminated string otherwise.
 *
 * If the string to be written to \p err is larger than \p errsize then \p err
 * will be populated as much as possible (and null-terminated). However, as
 * opposed to other API functions, the functions returns the original error.
 *
 * \param[in]  config  The configuration to update
 * \param[in]  argc    Number strings in \p argv
 * \param[in]  argv    Strings given on the command line
 * \param[in]  options Options to provide on the command line (see \ref CLO)
 * \param[out] err     Pointer to a char array for storing an error description
 *                     (can be set to NULL)
 * \param[in]  errsize Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationUpdateFromCommandLine(void* config, int argc,
                                                     const char* const* argv,
                                                     int options, char* err,
                                                     int errsize);

/*!
 * Updates a configuration from a JSON-formatted string.
 *
 * The function populates \p err with:
 * - a description of the error if the function returns one of
 *   #YOGI_ERR_PARSING_JSON_FAILED or #YOGI_ERR_CONFIG_NOT_VALID; and
 * - an empty, null-terminated string otherwise.
 *
 * If the string to be written to \p err is larger than \p errsize then \p err
 * will be populated as much as possible (and null-terminated). However, as
 * opposed to other API functions, the functions returns the original error.
 *
 * \param[in]  config  The configuration to update
 * \param[in]  json    Null-terminated, JSON-formatted string
 * \param[out] err     Pointer to a char array for storing an error description
 *                     (can be set to NULL)
 * \param[in]  errsize Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationUpdateFromJson(void* config, const char* json,
                                              char* err, int errsize);

/*!
 * Updates a configuration from a JSON file.
 *
 * The function populates \p err with:
 * - a description of the error if the function returns one of
 *   #YOGI_ERR_PARSING_FILE_FAILED or #YOGI_ERR_CONFIG_NOT_VALID; and
 * - an empty, null-terminated string otherwise.
 *
 * If the string to be written to \p err is larger than \p errsize then \p err
 * will be populated as much as possible (and null-terminated). However, as
 * opposed to other API functions, the functions returns the original error.
 *
 * \param[in]  config   The configuration to update
 * \param[in]  filename Path to the JSON file
 * \param[out] err      Pointer to a char array for storing an error description
 *                      (can be set to NULL)
 * \param[in]  errsize  Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationUpdateFromFile(void* config,
                                              const char* filename, char* err,
                                              int errsize);

/*!
 * Retrieves a configuration as a JSON-formatted string.
 *
 * If the configuration is larger than the space provided by \p json and
 * \p jsonsize then this function returns #YOGI_ERR_BUFFER_TOO_SMALL, however,
 * \p str will still be populated as much as possible (and null-terminated). In
 * this case, \p json will *not* contain valid JSON data.
 *
 * \param[in]  config   The configuration
 * \param[out] json     Pointer to a string for storing the configuration
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 * \param[in]  resvars  Set to #YOGI_TRUE to resolve any variables before
 *                      writing the configuration to \p json and #YOGI_FALSE
 *                      otherwise
 * \param[in]  indent   Indentation size (number of space characters to use);
 *                      -1 omits new lines as well
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationDump(void* config, char* json, int jsonsize,
                                    int resvars, int indent);

/*!
 * Writes a configuration to a file in JSON format.
 *
 * This is useful for debugging purposes.
 *
 * \param[in] config   The configuration
 * \param[in] filename Path to the output file
 * \param[in] resvars  Set to #YOGI_TRUE to resolve any variables before writing
 *                     the configuration to the file and #YOGI_FALSE otherwise
 * \param[in] indent   Indentation size (number of space characters to use);
 *                     -1 omits new lines as well
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationWriteToFile(void* config, const char* filename,
                                           int resvars, int indent);

/*!
 * Validates the configuration against a JSON Schema.
 *
 * The validation is based on JSON Schema draft-07, see http://json-schema.org/.
 * The schema to validate \p config against has to be supplied in \p schema
 * which needs to be a configuration object itself.
 *
 * If the validation fails, #YOGI_ERR_CONFIGURATION_VALIDATION_FAILED will be
 * returned and \p err will be populated with a human-readable description
 * about the failure.
 *
 * \param[in]  config   The configuration to validate
 * \param[in]  section  Section in \p config to validate (set to NULL for root);
 *                      syntax is JSON pointer (RFC 6901)
 * \param[in]  schema   The configuration containing the schema
 * \param[out] err      Pointer to a char array for storing an error description
 *                      (can be set to NULL)
 * \param[in]  errsize  Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ConfigurationValidate(void* config, const char* section,
                                        void* schema, char* err, int errsize);

/*!
 * Creates a context for the execution of asynchronous operations.
 *
 * \param[out] context Pointer to the context handle
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextCreate(void** context);

/*!
 * Runs the context's event processing loop to execute ready handlers.
 *
 * This function runs handlers (internal and user-supplied such as functions
 * registered through YOGI_ContextPost()) that are ready to run, without
 * blocking, until the YOGI_ContextStop() function has been called or there
 * are no more ready handlers.
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in]  context The context to use
 * \param[out] count   Number of executed handlers (may be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextPoll(void* context, int* count);

/*!
 * Runs the context's event processing loop to execute at most one ready
 * handler.
 *
 * This function runs at most one handler (internal and user-supplied such as
 * functions registered through YOGI_ContextPost()) that is ready to run,
 * without blocking.
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in]  context The context to use
 * \param[out] count   Number of executed handlers (may be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextPollOne(void* context, int* count);

/*!
 * Runs the context's event processing loop for the specified duration.
 *
 * This function blocks while running the context's event processing loop and
 * calling dispatched handlers (internal and user-supplied such as functions
 * registered through YOGI_ContextPost()) for the specified duration unless
 * YOGI_ContextStop() is called within that time.
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in]  context  The context to use
 * \param[out] count    Number of executed handlers (may be set to NULL)
 * \param[in]  duration Duration in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextRun(void* context, int* count, long long duration);

/*!
 * Runs the context's event processing loop for the specified duration to
 * execute at most one handler.
 *
 * This function blocks while running the context's event processing loop and
 * calling dispatched handlers (internal and user-supplied such as functions
 * registered through YOGI_ContextPost()) for the specified duration until a
 * single handler function has been executed, unless YOGI_ContextStop() is
 * called within that time.
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in]  context  The context to use
 * \param[out] count    Number of executed handlers (may be set to NULL)
 * \param[in]  duration Duration in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextRunOne(void* context, int* count, long long duration);

/*!
 * Starts an internal thread for running the context's event processing loop.
 *
 * This function starts a threads that runs the context's event processing loop
 * in the background. It relieves the user from having to start a thread and
 * calling the appropriate YOGI_ContextRun... or YOGI_ContextPoll... functions
 * themself. The thread can be stopped using YOGI_ContextStop().
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in] context The context to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextRunInBackground(void* context);

/*!
 * Stops the context's event processing loop.
 *
 * This function signals the context to stop running its event processing loop.
 * This causes YOGI_ContextRun... functions to return as soon as possible and it
 * terminates the thread started via YOGI_ContextRunInBackground().
 *
 * \param[in] context The context to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextStop(void* context);

/*!
 * Blocks until the context's event processing loop is being run or until the
 * specified timeout is reached.
 *
 * If the \p duration parameter is set to 0 then the function works in polling
 * mode.
 *
 * If the event processing loop has not been running after the specified
 * timeout, then the #YOGI_ERR_TIMEOUT error is returned. This also applies when
 * this function is used in polling mode as described above.
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in] context  The context to use
 * \param[in] duration Maximum time to wait in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextWaitForRunning(void* context, long long duration);

/*!
 * Blocks until no thread is running the context's event processing loop or
 * until the specified timeout is reached.
 *
 * If the \p duration parameter is set to 0 then the function works in polling
 * mode.
 *
 * If a thread is still running the event processing loop after the specified
 * timeout, then the #YOGI_ERR_TIMEOUT error is returned. This also applies when
 * this function is used in polling mode as described above.
 *
 * This function must be called from outside any handler functions that are
 * being executed through the context.
 *
 * \param[in] context  The context to use
 * \param[in] duration Maximum time to wait in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextWaitForStopped(void* context, long long duration);

/*!
 * Adds the given function to the context's event processing queue to be
 * executed and returns immediately.
 *
 * The handler \p fn will only be executed after this function returns and only
 * by a thread running the context's event processing loop. The only parameter
 * for \p fn will be set to the value of the \p userarg parameter.
 *
 * \param[in] context The context to use
 * \param[in] fn      The function to call from within the given context
 * \param[in] userarg User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_ContextPost(void* context, void (*fn)(void* userarg),
                              void* userarg);

/*!
 * Raises a signal.
 *
 * Signals in Yogi are intended to be used similar to POSIX signals. They have
 * to be raised explicitly by the user (e.g. when receiving an actual POSIX
 * signal like SIGINT) via this function. A signal will be received by all
 * signal sets containing that signal.
 *
 * The \p sigarg parameter can be used to deliver user-defined data to the
 * signal handlers. The cleanup handler \p fn will be called once all signal
 * handlers have been called; it is intended to be used for memory management
 * purposes. Once \p fn has been called, \p sigarg is not used any more and
 * can be destroyed.
 *
 * \note
 *   The cleanup handler \p fn can get called either from within the
 *   YOGI_RaiseSignal() function or from any context within the program.
 *
 * \param[in] signal  The signal to raise (see \ref SIG)
 * \param[in] sigarg  User-defined data to pass to the signal handlers
 * \param[in] fn      Function to be called once all signal handlers have been
 *                    executed (can be NULL)
 * \param[in] userarg User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_RaiseSignal(int signal, void* sigarg,
                              void (*fn)(void* sigarg, void* userarg),
                              void* userarg);

/*!
 * Creates a new signal set.
 *
 * Signal sets are used to receive signals raised via YOGI_RaiseSignal().
 * The signals are queued until they can be delivered by means of calls to
 * YOGI_SignalSetAwaitSignalAsync().
 *
 * \param[out] sigset  Pointer to the signal set handle
 * \param[in]  context The context to use
 * \param[in]  signals The signals to listen for
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_SignalSetCreate(void** sigset, void* context, int signals);

/*!
 * Waits for a signal to be raised.
 *
 * The handler \p fn will be called after one of the signals in the signal set
 * is caught. The parameters passed to \p fn are:
 *  -# __res__: #YOGI_OK or error code in case of a failure (see \ref EC)
 *  -# __sig__: The caught signal (see \ref SIG)
 *  -# __sigarg__: User-defined parameter passed to YOGI_RaiseSignal()
 *  -# __userarg__: Value of the user-specified \p userarg parameter
 *
 * \note
 *   Calling this function on the same context again before the signal has been
 *   caught will cause the previously registered handler function to be called
 *   with the #YOGI_ERR_CANCELED error.
 *
 * \param[in] sigset  The signal set
 * \param[in] fn      The function to call
 * \param[in] userarg User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_SignalSetAwaitSignalAsync(
    void* sigset, void (*fn)(int res, int sig, void* sigarg, void* userarg),
    void* userarg);

/*!
 * Cancels waiting for a signal.
 *
 * Causes the handler function registered via YOGI_SignalSetAwaitSignalAsync()
 * to be called with #YOGI_ERR_CANCELED.
 *
 * \note
 *   If the receive handler has already been scheduled for execution, this
 *   function will fail with the #YOGI_ERR_OPERATION_NOT_RUNNING error.
 *
 * \param[in] sigset The signal set
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_SignalSetCancelAwaitSignal(void* sigset);

/*!
 * Creates a new timer.
 *
 * \param[out] timer   Pointer to the timer handle
 * \param[in]  context The context to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_TimerCreate(void** timer, void* context);

/*!
 * Starts the given timer.
 *
 * If the timer is already running, the timer will be canceled first, as if
 * YOGI_TimerCancel() were called explicitly.
 *
 * The parameters of the handler function \p fn are:
 *  -# __res__: #YOGI_OK or error code in case of a failure (see \ref EC)
 *  -# __userarg__: Value of the user-specified \p userarg parameter
 *
 * \param[in] timer    The timer to start
 * \param[in] duration Duration in nanoseconds (-1 for infinity)
 * \param[in] fn       The function to call after the given time passed
 * \param[in] userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_TimerStartAsync(void* timer, long long duration,
                                  void (*fn)(int res, void* userarg),
                                  void* userarg);

/*!
 * Cancels the given timer.
 *
 * Canceling a timer will result in the handler function registered via
 * YOGI_TimerStartAsync() to be called with the #YOGI_ERR_CANCELED error as
 * first parameter. Note that if the handler is already scheduled for executing,
 * it will be called with #YOGI_OK instead.
 *
 * If the timer has not been started or it already expired, this function will
 * return #YOGI_ERR_TIMER_EXPIRED.
 *
 * \param[in] timer The timer to cancel
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_TimerCancel(void* timer);

/*!
 * Creates a new branch.
 *
 * A branch represents an entry point into a Yogi network. It advertises itself
 * via IP broadcasts/multicasts with its unique ID and information required for
 * establishing a connection. If a branch detects other branches on the network,
 * it connects to them via TCP to retrieve further information such as their
 * name, description and network name. If the network names match, two branches
 * attempt to authenticate with each other by securely comparing passwords.
 * Once authentication succeeds and there is no other known branch with the same
 * path then the branches can actively communicate as part of the Yogi network.
 *
 * The branch is configured via the \p config parameter. The configuration
 * object will only be used while constructing the branch, i.e. the branch will
 * not keep any references to it. The supplied configuration must have the
 * following structure:
 *
 * \code
 *   {
 *     "name":                   "Fan Controller",
 *     "description":            "Controls a fan via PWM",
 *     "path":                   "/Cooling System/Fan Controller",
 *     "network_name":           "Hardware Control",
 *     "network_password":       "secret",
 *     "advertising_interfaces": ["localhost"],
 *     "advertising_address":    "ff02::8000:2439",
 *     "advertising_port":       13531,
 *     "advertising_interval":   1.0,
 *     "timeout":                3.0,
 *     "ghost_mode":             false,
 *     "tx_queue_size":          1000000,
 *     "rx_queue_size":          100000
 *   }
 * \endcode
 *
 * All of the properties are optional and if unspecified (or set to _null_),
 * their respective default values will be used (see \ref CV). The properties
 * have the following meaning:
 *  - __name__: Name of the branch.
 *  - __description__: Description of the branch.
 *  - __path__: Path of the branch in the network. Must start with a slash.
 *  - __network_name__: Name of the network to join.
 *  - __network_password__: Password for the network.
 *  - __advertising_interfaces__: Network interfaces to use for advertising and
 *    for branch connections. Valid strings are Unix device names ("eth0",
 *    "en5", "wlan0"), adapter names on Windows ("Ethernet",
 *    "VMware Network Adapter WMnet1") or MAC addresses ("11:22:33:44:55:66").
 *    Furthermore, the special strings "localhost" and "all" can be used to
 *    denote loopback and all available interfaces respectively.
 *  - __advertising_address__: Multicast address to use for advertising, e.g.
 *    239.255.0.1 for IPv4 or ff02::8000:1234 for IPv6.
 *  - __advertising_port__: Port to use for advertising.
 *  - __advertising_interval__: Time between advertising messages. Must be at
 *    least 1 ms.
 *  - __timeout__: Amount of time of inactivity before a connection is
 *    considered to be broken. Must be at least 1 ms.
 *  - __ghost_mode__: Set to true to activate ghost mode.
 *  - __tx_queue_size__: Size of the send queues for remote branches.
 *  - __rx_queue_size__: Size of the receive queues for remote branches.
 *
 * Advertising and establishing connections can be limited to certain network
 * interfaces via the _interface_ property. The default is to use all
 * available interfaces.
 *
 * Setting the _ghost_mode_ property to _true_ prevents the branch from actively
 * participating in the Yogi network, i.e. the branch will not advertise itself
 * and it will not authenticate in order to join a network. However, the branch
 * will temporarily connect to other branches in order to obtain more detailed
 * information such as name, description, network name and so on. This is useful
 * for obtaining information about active branches without actually becoming
 * part of the Yogi network.
 *
 * \note
 *   Even if the communication between branches is not encrypted, the
 *   authentication process via passwords is always done in a secure manner.
 *
 * \attention
 *   The _tx_queue_size_ and _rx_queue_size_ properties affect every branch
 *   connection and can therefore consume a large amount of memory. For example,
 *   in a network of 10 branches where these properties are set to 1 MB, the
 *   resulting memory used for the queues would be 10 x 2 x 1 MB = 20 MB for
 *   each of the 10 branches. This value grows with the number of branches
 *   squared.
 *
 * \param[out] branch   Pointer to the branch handle
 * \param[in]  context  The context to use
 * \param[in]  config   Branch properties (set to NULL to use defaults)
 * \param[in]  section  Section in \p config to use (set to NULL for root);
 *                      syntax is JSON pointer (RFC 6901)
 * \param[out] err      Pointer to a char array for storing an error description
 *                      (can be set to NULL)
 * \param[in]  errsize  Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchCreate(void** branch, void* context, void* config,
                               const char* section, char* err, int errsize);

/*!
 * Retrieves information about a local branch.
 *
 * This function writes the branch's UUID (16 bytes) in binary form to \p uuid.
 * Any further information is written to \p json in JSON format. The function
 * call fails with the #YOGI_ERR_BUFFER_TOO_SMALL error if the produced JSON
 * string does not fit into \p json, i.e. if \p jsonsize is too small. However,
 * in that case, the first \p jsonsize - 1 characters and a trailing zero will
 * be written to \p json.
 *
 * The produced JSON string is as follows, without any unnecessary whitespace:
 *
 * \code
 *   {
 *     "uuid":                   "6ba7b810-9dad-11d1-80b4-00c04fd430c8",
 *     "name":                   "Fan Controller",
 *     "description":            "Controls a fan via PWM",
 *     "network_name":           "Hardware Control",
 *     "path":                   "/Cooling System/Fan Controller",
 *     "hostname":               "beaglebone",
 *     "pid":                    4124,
 *     "advertising_interfaces": ["localhost"],
 *     "advertising_address":    "ff02::8000:2439",
 *     "advertising_port":       13531,
 *     "advertising_interval":   1.0,
 *     "tcp_server_address":     "::",
 *     "tcp_server_port":        53332,
 *     "start_time":             "2018-04-23T18:25:43.511Z",
 *     "timeout":                3.0,
 *     "ghost_mode":             false
 *   }
 * \endcode
 *
 * \param[in]  branch   The branch handle
 * \param[out] uuid     Pointer to a 16 byte array for storing the UUID (can be
 *                      set to NULL)
 * \param[out] json     Pointer to a char array for storing the information (can
 *                      be set to NULL)
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchGetInfo(void* branch, void* uuid, char* json,
                                int jsonsize);

/*!
 * Retrieves information about all connected remote branches.
 *
 * For each of the connected remote branches, this function will:
 *  -# Write the branch's UUID (16 bytes) in binary form to \p uuid.
 *  -# Generate a JSON string containing further information to \p json.
 *  -# Execute the handler \p fn with #YOGI_OK as first argument if \p jsonsize
 *     is as least as large as the length of the generated JSON string
 *
 * If the produced JSON string for the branch does not fit into \p json, i.e. if
 * \p jsonsize is too small, then \p json will be filled with the first
 * \p jsonsize - 1 characters and a trailing zero and \p fn will be called with
 * the #YOGI_ERR_BUFFER_TOO_SMALL error for that particular branch.
 *
 * This function will return #YOGI_ERR_BUFFER_TOO_SMALL if \p json is not large
 * enough to hold each one of the JSON strings. However, \p fn will still be
 * called for each discovered branch.
 *
 * The produced JSON string is as follows, without any unnecessary whitespace:
 *
 * \code
 *   {
 *     "uuid":                 "123e4567-e89b-12d3-a456-426655440000",
 *     "name":                 "Pump Safety Logic",
 *     "description":          "Monitors the pump for safety",
 *     "network_name":         "Hardware Control",
 *     "path":                 "/Cooling System/Pump/Safety",
 *     "hostname":             "beaglebone",
 *     "pid":                  3321,
 *     "tcp_server_address":   "fe80::f086:b106:2c1b:c45",
 *     "tcp_server_port":      43384,
 *     "start_time":           "2018-04-23T18:25:43.511Z",
 *     "timeout":              3.0,
 *     "advertising_interval": 1.0,
 *     "ghost_mode":           false
 *   }
 * \endcode
 *
 * \param[in]  branch   The branch handle
 * \param[out] uuid     Pointer to 16 byte array for storing the UUID (can be
 *                      set to NULL)
 * \param[out] json     Pointer to a char array for storing the information (can
 *                      be set to NULL)
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 * \param[in]  fn       Handler to call for each connected branch
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchGetConnectedBranches(void* branch, void* uuid,
                                             char* json, int jsonsize,
                                             void (*fn)(int res, void* userarg),
                                             void* userarg);

/*!
 * Wait for a branch event to occur.
 *
 * This function will register \p fn to be called once one of the given branch
 * \p events occurs on the given \p branch. The parameters passed to \p fn are:
 *  -# __res__: #YOGI_OK or error code associated with the wait operation
 *  -# __event__: The branch event that occurred (see \ref BEV)
 *  -# __evres__: #YOGI_OK or error code associated with the event
 *  -# __userarg__: Value of the user-specified \p userarg parameter
 *
 * If this function is called while a previous wait operation is still active
 * then the previous operation will be canceled, i.e. \p fn for the previous
 * operation will be called with the #YOGI_ERR_CANCELED error.
 *
 * The \p uuid parameter will be populated with the UUID of the branch that
 * caused the event, i.e. if the remote branch B gets discovered, causing the
 * #YOGI_BEV_BRANCH_DISCOVERED event to be generated, then \p uuid will be
 * populated with B's UUID.
 *
 * The \p json parameter will be populated with a string in JSON format
 * containing additional event information such as branch information See
 * \ref BEV for event-specific details.
 *
 * If the produced JSON string for the branch does not fit into \p json, i.e. if
 * \p jsonsize is too small, then \p json will be filled with the first
 * \p jsonsize - 1 characters and a trailing zero and \p fn will be called with
 * the #YOGI_ERR_BUFFER_TOO_SMALL error for that particular branch.
 *
 * \attention
 *   Make sure that the two supplied buffers \p uuid and \p json remain valid
 *   until \p fn has been executed.
 *
 * \param[in]  branch   The branch handle
 * \param[in]  events   Events to observe (see \ref BEV)
 * \param[out] uuid     Pointer to 16 byte array for storing the UUID
 *                      (can be set to NULL)
 * \param[out] json     Pointer to a char array for storing event information
 *                      (can be set to NULL)
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 * \param[in]  fn       Handler to call for the received event
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchAwaitEventAsync(
    void* branch, int events, void* uuid, char* json, int jsonsize,
    void (*fn)(int res, int event, int evres, void* userarg), void* userarg);

/*!
 * Cancels waiting for a branch event.
 *
 * Calling this function will cause the handler registered via
 * YOGI_BranchAwaitEventAsync() to be called with the #YOGI_ERR_CANCELED error.
 *
 * \note
 *   If the receive handler has already been scheduled for execution, this
 *   function will fail with the #YOGI_ERR_OPERATION_NOT_RUNNING error.
 *
 * \param[in] branch The branch handle
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchCancelAwaitEvent(void* branch);

/*!
 * Sends a broadcast message to all connected branches.
 *
 * Broadcast messages contain arbitrary data encoded as JSON or MessagePack. As
 * opposed to sending messages via terminals, broadcast messages don't have to
 * comply with a defined schema for the payload; any data that can be encoded
 * is valid. This implies that validating the data is entirely up to the user
 * code.
 *
 * \note
 *   The payload in \p data can be given encoded in either JSON or MessagePack
 *   as specified in the \p datafmt parameter. It does not matter which format
 *   is chosen since the receivers can specify their desired format and the
 *   library performs the necessary conversions automatically.
 *
 * Setting the \p block parameter to #YOGI_FALSE will cause the function to skip
 * sending the message to branches that have a full send queue. If at least one
 * branch was skipped, the function will return the #YOGI_ERR_TX_QUEUE_FULL
 * error. If the parameter is set to #YOGI_TRUE instead, the function will block
 * until the message has been put into the send queues of all connected
 * branches.
 *
 * \attention
 *   Calling this function from within a handler function executed through the
 *   branch's _context_  with \p block set to #YOGI_TRUE will cause a dead-lock
 *   if any send queue is full!
 *
 * \param[in] branch   The branch handle
 * \param[in] enc      Encoding type used for \p data (see \ref ENC)
 * \param[in] data     Payload encoded according to \p datafmt
 * \param[in] datasize Number of bytes in \p data
 * \param[in] block    Block until message has been put into all send buffers
 *                     (#YOGI_TRUE or #YOGI_FALSE)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchSendBroadcast(void* branch, int enc, const void* data,
                                      int datasize, int block);

/*!
 * Sends a broadcast message to all connected branches.
 *
 * Broadcast messages contain arbitrary data encoded as JSON or MessagePack. As
 * opposed to sending messages via terminals, broadcast messages don't have to
 * comply with a defined schema for the payload; any data that can be encoded
 * is valid. This implies that validating the data is entirely up to the user
 * code.
 *
 * \note
 *   The payload in \p data can be given encoded in either JSON or MessagePack
 *   as specified in the \p datafmt parameter. It does not matter which format
 *   is chosen since the receivers can specify their desired format and the
 *   library performs the necessary conversions automatically.
 *
 * The handler function \p fn will be called once the operation finishes. Its
 * parameters are:
 *  -# __res__: #YOGI_OK or error code associated with the operation
 *  -# __oid__: Operation ID as returned by this library function
 *  -# __userarg__: Value of the user-specified \p userarg parameter
 *
 * Setting the \p retry parameter to #YOGI_FALSE will cause the function to skip
 * sending the message to branches that have a full send queue. If at least one
 * branch was skipped, the handler \p fn will be called with the
 * #YOGI_ERR_TX_QUEUE_FULL error. If the parameter is set to #YOGI_TRUE instead,
 * \p fn will be called once the message has been put into the send queues of
 * all connected branches.
 *
 * The function returns an ID which uniquely identifies this send operation
 * until \p fn has been called. It can be used in a subsequent
 * YOGI_BranchCancelSendBroadcast() call to abort the operation.
 *
 * \note
 *   The memory pointed to via \p data will be copied if necessary, i.e. \p data
 *   only needs to remain valid until the function returns.
 *
 * \param[in] branch   The branch handle
 * \param[in] enc      Encoding type used for \p data (see \ref ENC)
 * \param[in] data     Payload encoded according to \p datafmt
 * \param[in] datasize Number of bytes in \p data
 * \param[in] retry    Retry sending the message (#YOGI_TRUE or #YOGI_FALSE)
 * \param[in] fn       Handler to call once the operation finishes
 * \param[in] userarg  User-specified argument to be passed to \p fn
 *
 * \returns [>0] Operation ID if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchSendBroadcastAsync(
    void* branch, int enc, const void* data, int datasize, int retry,
    void (*fn)(int res, int oid, void* userarg), void* userarg);

/*!
 * Cancels a send broadcast operation.
 *
 * Calling this function will cause the send operation with the specified
 * operation ID \p oid to be canceled, resulting in the handler function
 * registered via the YOGI_BranchSendBroadcastAsync() call that returned the
 * same \p oid to be called with the #YOGI_ERR_CANCELED error.
 *
 * \note
 *   If the send operation has already been carried out but the handler function
 *   has not been called yet, then cancelling the operation will fail and the
 *   #YOGI_ERR_INVALID_OPERATION_ID will be returned.
 *
 * \param[in] branch The branch handle
 * \param[in] oid    Operation ID of the send operation to cancel
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchCancelSendBroadcast(void* branch, int oid);

/*!
 * Receives a broadcast message from any of the connected branches.
 *
 * Broadcast messages contain arbitrary data encoded as JSON or MessagePack. As
 * opposed to sending messages via terminals, broadcast messages don't have to
 * comply with a defined schema for the payload; any data that can be encoded
 * is valid. This implies that validating the data is entirely up to the user
 * code.
 *
 * \note
 *   The desired encoding of the received payload can be set via \p datafmt.
 *   The library will automatically perform any necessary conversions.
 *
 * This function will register \p fn to be called once a broadcast message has
 * been received. The parameters passed to \p fn are:
 *  -# __res__: #YOGI_OK or error code in case of a failure (see \ref EC)
 *  -# __size__: Number of bytes written to \p data
 *  -# __userarg__: Value of the user-specified \p userarg parameter
 *
 * If the received payload does not fit into \p data, i.e. if \p datasize is too
 * small, then \p fn will be called with the #YOGI_ERR_BUFFER_TOO_SMALL error
 * after populating \p data
 *  - with the first \p datasize - 1 characters of the received payload plus a
 *    trailing zero if \p datafmt is #YOGI_ENC_JSON; and
 *  - with the first \p datasize bytes of the received payload if \p datafmt is
 *    #YOGI_ENC_MSGPACK.
 *
 * If this function is called while a previous receive operation is still active
 * then the previous operation will be canceled with the #YOGI_ERR_CANCELED
 * error.
 *
 * \attention
 *   Broadcast messages do not get queued, i.e. if a branch is not actively
 *   receiving broadcast messages then they will be discarded. To ensure that
 *   no messages get missed, call YOGI_BranchReceiveBroadcastAsync() again from
 *   within the handler \p fn.
 *
 * \param[in]  branch   The branch handle
 * \param[out] uuid     Pointer to a 16 byte array for storing the UUID of the
 *                      sending branch (can be set to NULL)
 * \param[in]  enc      Encoding type to use for \p data (see \ref ENC)
 * \param[out] data     Pointer to a buffer to store the received payload in
 * \param[in]  datasize Maximum number of bytes to write to \p data
 * \param[in]  fn       Handler to call for the received broadcast message
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchReceiveBroadcastAsync(
    void* branch, void* uuid, int enc, void* data, int datasize,
    void (*fn)(int res, int size, void* userarg), void* userarg);

/*!
 * Cancels receiving a broadcast message.
 *
 * Calling this function will cause the handler registered via
 * YOGI_BranchReceiveBroadcastAsync() to be called with the #YOGI_ERR_CANCELED
 * error.
 *
 * \note
 *   If the receive handler has already been scheduled for execution, this
 *   function will fail with the #YOGI_ERR_OPERATION_NOT_RUNNING error.
 *
 * \param[in] branch The branch handle
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchCancelReceiveBroadcast(void* branch);

/*!
 * Destroys an object.
 *
 * Tries to destroy the object belonging to the given handle. The call fails and
 * returns #YOGI_ERR_OBJECT_STILL_USED if the object is still being used by
 * other objects that have been created via other library calls.
 *
 * Never destroy a context object from within a handler function that is
 * executed through the same context.
 *
 * Destroying an object will cause any active asynchronous operations to get
 * canceled and the corresponding completion handlers will be invoked with an
 * error code of #YOGI_ERR_CANCELED.
 *
 * \param[in] object The object to destroy
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_Destroy(void* object);

/*!
 * Destroys all objects.
 *
 * Destroys all previously created objects. All handles will be invalidated and
 * must not be used any more. This effectively resets the library.
 *
 * This function must be called outside of any handler function that is executed
 * through a context's event loop.
 *
 * Destroying objects will cause any active asynchronous operations to get
 * canceled and the corresponding completion handlers will be invoked with an
 * error code of #YOGI_ERR_CANCELED.
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_DestroyAll();

/*!
 * Creates a web server.
 *
 * Apart from serving static content from files and content from user-defined
 * routes, this web server controls a single branch that can be accessed via
 * JavaScript on a web page. The web server is sufficient for simple web
 * applications. However, more complex applications can utilize a more
 * sophisticated web server and use this built-in web server only as a means to
 * access the Yogi network.
 *
 * For security reasons and browser security restrictions, connections are
 * served over HTTPS/WSS only.
 *
 * The web server supports the following core features:
 * - Support for both IPv4 and IPv6 clients on the same socket
 * - Serving static content from files with configurable routes
 * - Serving user-generated content over HTTPS
 * - Long-running server-side processes controlled by the client
 * - User authentication and access control for routes and resources
 * - Per-user persistant storage
 *
 * The web server is configured via the \p config parameter. The configuration
 * object will only be used while constructing the server, i.e. the server will
 * not keep any references to it. While none of the properties are mandatory the
 * following excerpt shows the default values of the core parameters:
 *
 * \code
 *   {
 *     "port":            8443,
 *     "interfaces":      ["localhost"],
 *     "timeout":         30.0,
 *     "test_mode":       false,
 *     "compress_assets": true,
 *     "cache_size":      100000000,
 *     "routes": {
 *       "/": {
 *         "type":        "content",
 *         "mime":        "text/html",
 *         "permissions": { "*": ["GET"] },
 *         "enabled":     true,
 *         "content": [
 *           "<!DOCTYPE html>",
 *           "<html>",
 *           "<body>",
 *           "  <h1>Welcome to the Yogi web server!</h1>",
 *           "</body>",
 *           "</html>"
 *         ]
 *       }
 *     },
 *     "api_permissions": {
 *       "/auth/session":       { "*": ["POST", "DELETE"] },
 *       "/auth/groups":        { "*": ["GET"] },
 *       "/auth/groups/ *":"    { "*": ["GET"] },
 *       "/auth/users":         { "*": ["GET"] },
 *       "/auth/users/ *":      { "users": ["GET"], "owner": ["PATCH"] },
 *       "/branch/info":        { "users": ["GET"] },
 *       "/branch/connections": { "users": ["GET"] },
 *       "/branch/broadcasts":  { "users": ["GET", "POST"] }
 *     },
 *     "authentication": {
 *       "provider": "config",
 *       "readonly": false,
 *       "users": {
 *         "admin": {
 *           "first_name": "Administrator",
 *           "last_name":  "",
 *           "email":      "",
 *           "phone":      "",
 *           "password":
 * "8C6976E5B5410415BDE908BD4DEE15DFB167A9C873FC4BB8A81F6F2AB448A918",
 *           "groups":     ["admins", "users"],
 *           "enabled":    true
 *         }
 *       },
 *       "groups": {
 *         "admins": {
 *           "name":         "Administrators",
 *           "description":  "Users with unrestricted access to everything",
 *           "unrestricted": true
 *         },
 *         "users": {
 *           "name":         "Users",
 *           "description":  "All registered users"
 *         }
 *       }
 *     },
 *     "ssl": {
 *       "private_key": [
 *         "-----BEGIN PRIVATE KEY-----",
 *         "MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDccyREZEfU28sW",
 *         "kdtTxK8XA5pMdAlulFTizhgh9KTs62uKgHWq2zx2ISlZ+4cQfhLTATZBVhwQVLuD",
 *         "RiatrDqPL/cN2m5XER/vFUJw3vJm2u7qKmJGWQ0i4j3O7Yle+uYJHn+35TIUfEX5",
 *         "CgYnpt65lAjfbN1tl2ACbESa4E2nymZMSyOzTFd8xlL+nID2eG3CvKEKsg/bzXIV",
 *         "kf6IlNfpu1tCyeGlwKYaNMymP4Rgp/8Y+Gdp2NZh1JOjqZ9Cavn2divMdEb856hx",
 *         "nnqmoR/IiRyXJU8BdY1x2+NjbBmdcMNHTGioqhmzny+L1Pdfq6KgN4sIj0dQqe2j",
 *         "OfpMvJR1AgMBAAECggEBAJTmSLbRLRt/IduqxQP9s23OByhgsAY6tmNDZVV6EvPb",
 *         "x5WTUucGkf4QRAf9NqtTHI/dH7Jd4TK+qLDwbyubSypv8WUwBptUe8dXCruGOyBx",
 *         "/DG6UwrFF3Rab/kWxFSdD0dVjFq/umXP0st1k9awhXu/m4ad16owNq8sReJ+YrVT",
 *         "ZTIgowAklZ9QKnOCjZ3gbDWS8nn6dgonwU0INJD7iKJ3Mw4rv7Q18/7G7T+YmwHO",
 *         "QdTy8M7MBLKu+ifQjgh9khFZZ8G0/jdV9ZkLbTNR/OfyO7XsPhWMQehRWryN9x2a",
 *         "TekT8K0xJRu6NytDvvdeb139JlQjH6DnVrtMj8I4R/kCgYEA+ADkhZnI3ceHS2Kf",
 *         "+kUbbOJI6/ncm9eHOZ/dEVyIK+iYJO9Eba2XqjIMXl6scZpxRAakHG/zRA23fc+c",
 *         "R0mKEIRxJz9kLxpocW8ib+0LIeb0XdK8xt6JabW4EOtLExu3lIXpa+hkH7xr30Bb",
 *         "+OQZdUpDkk6gBtKbk433w9WfQPcCgYEA447Pn2zgE+Oj8sHEp1qdYbj7CMo0JcQj",
 *         "u4mmb5+BT2FiAlmuetoFGo2/0uGm4ha4cFtj1u58y00Ai8c+dKOrv9LzDHe09jaD",
 *         "uGu4vbwCC0l8wiTKrz5m/Wl3oGAi2tWYUEpDyFBYj9yUEsOtZnhY8S0e2LbrPV6L",
 *         "IftsXcZs5vMCgYEAh3JtXK417zJ3KOvvdY9iwCACNaccHp0ixI0T/tRrppd86Mco",
 *         "t0IU0CZPbQcF5XG1JLL/1GwsD2Hycir+Lkzys7yx0/6iJ7ujiThqNXjgGJ77nXZo",
 *         "FbEamXpe0ha/xOrhY6OTbZTZgh+1RpPu50skwFNT/kckzTUfNQJXbERymtcCgYBn",
 *         "XTJwP1tVjXymQ2AZiK6eehEFgBvJ39s/T5pbb7+kGB8V5Di+MxRIaITuvHM6lYl3",
 *         "9/tU4QWsrzCFTZ7QCpx0SNUAH8wCXubcOYo0HRdsmxfhdCbhDV5o06R0I9cRQh6q",
 *         "seEyN1TTHCXilCHKvvXfr1gopoYaXTyj1fn2arCDAwKBgQDHc9V+9IpZ2F+ZwSYn",
 *         "didwWQfsAZx5EiXtnXnyx0hkNjgebFQAgDJY4vvRphaD0bumywfNMOz/59jjjVZg",
 *         "cWXs9H91XtA7cT7wJi+xiGe6R8zRcVPfImwQchSsCoMGuyLCygJTJPMzGT+YbpBA",
 *         "Umrn/FzslGZsXxwMCR0E8654Kw==",
 *         "-----END PRIVATE KEY-----"
 *       ],
 *       "certificate_chain": [
 *         "-----BEGIN CERTIFICATE-----",
 *         "MIICvjCCAaYCCQCL4CgivAdrbzANBgkqhkiG9w0BAQsFADAhMQswCQYDVQQGEwJV",
 *         "UzESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTE5MDUxODA3MDQyOFoXDTI5MDUxNTA3",
 *         "MDQyOFowITELMAkGA1UEBhMCVVMxEjAQBgNVBAMMCWxvY2FsaG9zdDCCASIwDQYJ",
 *         "KoZIhvcNAQEBBQADggEPADCCAQoCggEBANxzJERkR9TbyxaR21PErxcDmkx0CW6U",
 *         "VOLOGCH0pOzra4qAdarbPHYhKVn7hxB+EtMBNkFWHBBUu4NGJq2sOo8v9w3ablcR",
 *         "H+8VQnDe8mba7uoqYkZZDSLiPc7tiV765gkef7flMhR8RfkKBiem3rmUCN9s3W2X",
 *         "YAJsRJrgTafKZkxLI7NMV3zGUv6cgPZ4bcK8oQqyD9vNchWR/oiU1+m7W0LJ4aXA",
 *         "pho0zKY/hGCn/xj4Z2nY1mHUk6Opn0Jq+fZ2K8x0RvznqHGeeqahH8iJHJclTwF1",
 *         "jXHb42NsGZ1ww0dMaKiqGbOfL4vU91+roqA3iwiPR1Cp7aM5+ky8lHUCAwEAATAN",
 *         "BgkqhkiG9w0BAQsFAAOCAQEAQQezEoFlVkRjB7x1QljRUAVqUSPpk4bERkynB3Nb",
 *         "hajLKKwZMeO8F4GDkAnY2F7ZD6KPQkOlKMVJJVmtVi3OTvutDJnc8jDXZUyejWe+",
 *         "tSwLU2Uo7scPjYynyyPHcNkut+V7XjvhLJr267X0O6iymYnyJBGweVppwwgfxAmH",
 *         "Rzv2uFLf+U7odtQmSC1g/VdImDoJsfASqnycXcqkmiDfv6Pjqp1dvUm9aDCIFdkf",
 *         "DXShGXE+SVXQ61FVMhV62OsNY36mM5lR2kMXwgybRNMWla8Cmu8OhCkftOvVLdW2",
 *         "tAVd+K6fpZe/mdCCuN3pXCCqu/0vPlFoLNMGZrKbLU8W6Q==",
 *         "-----END CERTIFICATE-----"
 *       ],
 *       "dh_params": [
 *         "-----BEGIN DH PARAMETERS-----",
 *         "MIIBCAKCAQEA4iRXTwOzm97m19J21Pg67Vda5Ocw1PS6/CS+0ks//CpvNaYz0hJA",
 *         "YbvCPzQLmxYyUH8NwYutfNrESWtBFs3qEKiZ9zx09cpacXf/gw4VBfclIl2HAlNo",
 *         "5jWzh9VQBc3CxSNJqCRiJUvgnVCx1ec47cH3vkEucw0ewzxdwkpXgOGbja5BxCuO",
 *         "Mtwb+qTXm4XozdAPPWlwryFvwJL60pvtsF7f9S8xZHIe309yeCNnUajmqyKwdJ18",
 *         "P8RNYFHDe8vvaJ7/cnNCMqWFl16hVmBoIYy11H+R8WAphniJKV6fdGTs+7OLSc7Q",
 *         "4/QFcIxwOY+wIaH2OEuOxTrKjAivFrGz8wIBAg==",
 *         "-----END DH PARAMETERS-----"
 *       ]
 *     }
 *   }
 * \endcode
 *
 * The properties above have the following meaning:
 *  - __port__: HTTPS port to use.
 *  - __interfaces__: Network interfaces to use. Valid strings are Unix devices
 *    names ("eth5", "en5", "wlan0"), adapter names on Windows ("Ethernet",
 *    "WMware Network Adapter WMnet1") or MAC addresses ("11:22:33:44:55:66").
 *    Furthermore, the special strings "localhost" and "all" can be used to
 *    denote loopback and all available interfaces respectively.
 *  - __timeout__: Time of inactivity before terminating a client connection.
 *  - __test_mode__: Set to true to enable functionality for testing both server
 *    and client code. Should only be enabled during development.
 *  - __compress_assets__: Enable compression when transferring compressable,
 *    static assets such as HTML or JavaScript files.
 *  - __cache_size__: Size of the file cache in bytes. Used to keep assets in
 *    memory instead of reading them from disk to improve performance. A value
 *    of 0 disables the cache entirely.
 *
 * The sub-sections following the properties above are described in more detail
 * in the next paragraphs:
 *
 * The __routes__ section describes both the content for static routes and the
 * level of access that users have to static and custom routes. The excerpt
 * below shows how to configure the three different types of routes, namely
 * static routes mapping to the file system (_filesystem_), static routes with
 * their content in the configuration (_content_) and custom routes which are
 * created via the YOGI_WebRouteCreate() function (_custom_):
 *
 * \code
 *   "routes": {
 *     "/": {
 *       "type":        "filesystem",
 *       "path":        "www",
 *       "permissions": { "*": ["GET"] },
 *       "enabled":     true
 *     },
 *     "/secret": {
 *       "type":        "content",
 *       "mime":        "text/plain",
 *       "owner":       "luke",
 *       "permissions": { "users": ["GET"], "owner": ["PUT"] },
 *       "content":     "I am your father",
 *       "enabled":     true
 *     },
 *     "/messages": {
 *       "type":        "custom",
 *       "permissions": { "*": ["GET"], "users": ["POST"] }
 *     },
 *     "/messages/ *": {
 *       "type":        "custom",
 *       "permissions": { "*": ["GET"], "owner": ["DELETE"] },
 *       "enabled":     true
 *     }
 *   }
 * \endcode
 *
 * The _filesystem_ route above maps URIs to the server's root URI and below to
 * the files found in the file system under the (here relative to the current
 * working directory of the process) "www" directory. The optional _owner_
 * property assigns a user to _own_ that resource which can be used in the
 * _permissions_ section to grant specific permissions to that user. The
 * _permissions_ section describes the level of access that users belonging to
 * certain groups have. The _enabled_ property is also optional and can be used
 * to disable a route without removing it from the configuration. If this
 * property is not specified it is implicitly set to _true_.
 *
 * \note
 *   If a client issues a _GET_ request on a _filesystem_ route leading to a
 *   directory then the corresponding _index.html_ file in that directory will
 *   be delivered if it exists.
 *
 * A single "*" in the URI of a route is a whilecard for every immediate
 * sub-path while "**" applies to every path below, no matter how deep. Routes
 * that have more specific URIs override the properties inherited by routes
 * with less specific URIs, i.e. in the example above, the "/messages/ *" route
 * overrides the properties inherited from the "/messages" route for all paths
 * below "/messages".
 *
 * \attention
 *   There should not be a space in any of the ".../ *" strings in the
 *   configuration. The space has only been added to a avoid C compiler warnings
 *   about the start of a comment block inside a comment.
 *
 * The access control lists (ACLs) in the _permissions_ section consist of a
 * dictionary mapping groups to allowed request methods. There are two groups
 * that have a special meaning: "*" and "owner". The "*" group means every user,
 * logged in or not. The "owner" group means the user who _owns_ the resource
 * which can have a different meaning depending on the resource. For example, in
 * the "/secret" section the owner is explicitly set to the user "luke" which
 * while in the "/messages/ *" section the custom route could assign the owner
 * of a message to the user who created that message using a _POST_ request on
 * the "/messages" route.
 *
 * \note
 *   Users are granted the combination of all permissions that apply for them.
 *   For example, in the "/secret" route above, the _owner_ is set to the user
 *   _luke_. The permissions grant _GET_ access to everyone and _PUT_ access
 *   to the owner, i.e. _luke_. The end result is that _luke_ has both _GET_ and
 *   _PUT_ access to the route.
 *
 * The __api_permissions__ section sets the permissions for each built-in API
 * endpoints. Any API endpoints not listed here are inaccessible for all but
 * unrestricted users.
 *
 * The __authentication__ section configures the authentication service. The
 * default configuration (see above) uses the configuration itself to retrieve
 * user and group information. By default, only the _admin_ user and the two
 * groups _admins_ and _users_ are created. The _admin_ user's default password
 * is "admin", stored in the configuration as a hashed value using SHA-256. The
 * _unrestricted_ property in the _admins_ group section is optional and _false_
 * by default; setting it to _true_ grants the members of the group every
 * possible permission for every route and API endpoint without having to grant
 * those permissions explicitly in the relevant sections.
 *
 * The _readonly_ property can be used to write-protect the users and groups
 * database. Note that for the _config_ provider, setting _readonly_ to false
 * will save changes in memory only. Any changes will be lost once the server
 * gets destroyed.
 *
 * In order to use external files to store users and groups, the following
 * configuration can be used:
 *
 * \code
 *   "authentication": {
 *     "provider":    "files",
 *     "readonly":    false,
 *     "users_file":  "users.json",
 *     "groups_file": "groups.json"
 *   }
 * \endcode
 *
 * The external users and groups files have to be in JSON format and contain the
 * _users_ and _groups_ sections respectively directly under the root object,
 * with the same structure as the default configuration above.
 *
 * \note
 *   The _users_file_ and _groups_file_ can be set to the same file if having
 *   both users and groups in a single file is desired.
 *
 * The __ssl__ section configures the encryption of the HTTPS and WSS
 * connections. The private key, certificate chain and DH parameters can be put
 * directly into the server configuration as shown in the default configuration
 * above. However, in order to use external files, the following can be used:
 *
 * \code
 *   "ssl": {
 *     "private_key_file":       "key.pem",
 *     "private_key_password":   "password",
 *     "certificate_chain_file": "cert.pem",
 *     "dh_params_file":         "dh_params.pem"
 *   }
 * \endcode
 *
 * The external files have to use the PEM file format. If the private key file
 * is encrypted, its password has to be supplied in _private_key_password_.
 *
 * \attention
 *   Use the default private key only for testing since the key is publicly
 *   available! You can create your own key and a self-signed certificate using
 *   OpenSSL or services like letsencrypt.org.
 *
 * \note
 *   The DH parameters can be left at their default values since this does not
 *   pose a security issue.
 *
 * The supplied \p context will be used for housekeeping and for accepting
 * incoming connection requests. As long as no workers have been added using
 * YOGI_WebServerAddWorker(), \p context will also be used to handle client
 * requests.
 *
 * \param[out] server   Pointer to the server handle
 * \param[in]  context  The context to use (set to NULL to use the branch's
 *                      context)
 * \param[in]  branch   Branch to use (set to NULL to disable all
 *                      branch-specific features)
 * \param[in]  config   Server properties (set to NULL to use defaults)
 * \param[in]  section  Section in \p config to use (set to NULL for root);
 *                      syntax is JSON pointer (RFC 6901)
 * \param[out] err      Pointer to a char array for storing an error description
 *                      (can be set to NULL)
 * \param[in]  errsize  Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebServerCreate(void** server, void* context, void* branch,
                                  void* config, const char* section, char* err,
                                  int errsize);

/*!
 * Adds a worker thread to a web server.
 *
 * A worker represents a thread handling client requests. Multiple workers can
 * be added to a server in order to improve performance.
 *
 * \attention
 *   The user has to provide the thread running the provided context or call
 *   YOGI_ContextRunInBackground() in order to perform the actual work.
 *
 * \param[in] server  The server to add the worker to
 * \param[in] context The context to add as a worker
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebServerAddWorker(void* server, void* context);

/*!
 * Removes a worker thread from a web server.
 *
 * \param[in] server  The server that uses the context
 * \param[in] context The context to remove
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebServerRemoveWorker(void* server, void* context);

/*!
 * Creates a custom route.
 *
 * Registers a custom callback for handling requests on a custom URI.
 *
 * The handler function \p fn will be called on reception of a HTTP request from
 * a client. Its parameters are:
 *  -# __res__: #YOGI_OK or error code in case of a failure (see \ref EC)
 *  -# __rid__: Request ID to identify a new or running request
 *  -# __user__: User issuing the request or NULL if the client hasn't logged in
 *  -# __owner__: Owner as set in the configuration or NULL otherwise
 *  -# __method__: HTTP request method (see \ref MET)
 *  -# __uri__: The URI requested by the client without query parameters
 *  -# __params__: Query parameters; terminated by NULL as the last string
 *  -# __userarg__: Value of \p userarg
 *
 * Once a request has been received and \p fn has been called, one or more calls
 * to YOGI_WebRouteRespond() have to be made in order to respond to the client's
 * request. In case of connection errors such as a disconnected client or a
 * timeout \p will be called with a corresponding error and the _rid_ of the
 * request.
 *
 * \param[out] route    Pointer to the route handle
 * \param[in]  server   The server to use
 * \param[in]  baseuri  Base URI for the route (e.g. "/app/hello")
 * \param[in]  fn       Handler function to call for incoming HTTP requests
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebRouteCreate(
    void** route, void* server, const char* baseuri,
    void (*fn)(int res, int rid, const char* user, const char* owner,
               int method, const char* uri, const char* const* params,
               void* userarg),
    void* userarg);

/*!
 * Responds to a custom route request.
 *
 * This function must be called after the handler function registered via
 * YOGI_WebRouteCreate() has been invoked in order to respond to the client's
 * request.
 *
 * Calling this function multiple times will use chunked transfer encoding to
 * deliver the data to the client. The response is considered complete once
 * this function is called with \p finished set to #YOGI_TRUE.
 *
 * \param[in] route    The route to use
 * \param[in] rid      ID of the received request
 * \param[in] finished Response completed (#YOGI_TRUE or #YOGI_FALSE)
 * \param[in] status   HTTP status code to return (see \ref HTTP)
 * \param[in] contype  Mime type of the data in \p content (e.g. "text/plain")
 * \param[in] content  Content (body) of the response
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebRouteRespond(void* route, int rid, int finished,
                                  int status, const char* contype,
                                  const char* content);

/*!
 * Creates a client-controllable web process.
 *
 * A web process is a potentially long-running operation controlled by the
 * client. Examples are performing computationally expensive operations such as
 * image recognition or running external programs on the server.
 *
 * This function registers a handler function \p which is called whenever a
 * client starts a web process, sends input to the process or cancels it (see
 * \ref WPA). In order to send process output and status updates back to the
 * client, YOGI_WebProcessUpdate() has to be called.
 *
 * The handler function \p fn will be called upon the client's request:
 *  -# __res__: #YOGI_OK or error code in case of a failure (see \ref EC)
 *  -# __wpid__: Web process ID to identify a new or running process
 *  -# __user__: User issuing the request or NULL if the client hasn't logged in
 *  -# __wpa__: Requested web process action (see \WPA)
 *  -# __data__: Start of the received parameters or input data from the client
 *  -# __sizes__: Lenths of the individual data blocks (see below for details)
 *  -# __userarg__: Value of \p userarg
 *
 * The _data_ parameter contains either the parameters to start the process with
 * or input data to be sent to the process. The concept is the same as running
 * processes in a terminal: They are started with a set of parameters (i.e.
 * argc/argv) and input can be sent to them through the standard input (stdin).
 *
 * The _sizes_ is a -1-terminated array containing the sizes of the individual
 * data blocks in _data_. For input data, _sizes_ will only contain a single
 * size, e.g. for 10 bytes of received input data, _sizes_ will consist of the
 * values [10, -1]. For process parameters, _size_ will contain as many sizes
 * as parameters, e.g. for 3 received process parameters, _sizes_ consists of
 * values like [12, 34, 56, -1]. In this example, the first 12 bytes in _data_
 * make up the first parameter, the next 34 bytes make up the second parameter
 * and the following 56 bytes make up the third parameter.
 *
 * Having received a request and \p fn having been called, one or more calls to
 * YOGI_WebProcessUpdate() have to be made in order to inform the client about
 * new output from the process or a change in its state. In case of connection
 * errors such as a disconnected client \p will be called with a corresponding
 * error and the _wpid_ of the process.
 *
 * \param[out] wproc    Pointer to the web process handle
 * \param[in]  server   The server to use
 * \param[in]  name     Name (identifier) of the process (e.g. "echo")
 * \param[in]  fn       Handler function to call for incoming action requests
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebProcessCreate(void** wproc, void* server, const char* name,
                                   void (*fn)(int res, int wpid,
                                              const char* user, int wpa,
                                              const void* data,
                                              const int* sizes, void* userarg),
                                   void* userarg);

/*!
 * Sends output from a web process or a change in its state to the client.
 *
 * This function has to be called after having received a request from a client
 * to start a web process. It is used to inform the client about new output from
 * and process and changes in its state (see \ref WPU).
 *
 * For a new process, the first call to this function must be with \p wpu set to
 * either #YOGI_WPU_STARTED or #YOGI_WPU_FAILED_TO_START. If the process started
 * successfully, an arbitrary number of calls with \p wpu set to
 * #YOGI_WPU_OUTPUT can be made in order to send new process output to the
 * client. Once finished, the process calls this function once with either one
 * of #YOGI_WPU_FINISHED, #YOGI_WPU_CRASHED or #YOGI_WPU_CANCELED and the \p
 * exitcode optionally set to a value indicating the success or failure of the
 * process.
 *
 * Calling this function either one of #YOGI_WPU_FAILED_TO_START,
 * #YOGI_WPU_FINISHED, #YOGI_WPU_CRASHED or #YOGI_WPU_CANCELED will invalidate
 * the \p wpid and allow no further calls to this function for that particular
 * process.
 *
 * \param[in] wproc    The web process to use
 * \param[in] wpid     ID of the running web process
 * \param[in] wpu      Update type (see \ref WPU)
 * \param[in] exitcode Exit code of the process for indicating success/failure
 * \param[in] output   Output data (potantially binary) to send to the client
 * \param[in] outsize  Size of \p output in bytes
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_WebProcessUpdate(void* wproc, int wpid, int wpu, int exitcode,
                                   const void* output, int outsize);

//! @}

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // YOGI_CORE_H
