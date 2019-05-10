/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
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
//! @{

#define YOGI_HDR_VERSION "0.0.3"  ///< Whole version number
#define YOGI_HDR_VERSION_MAJOR 0  ///< Major version number
#define YOGI_HDR_VERSION_MINOR 0  ///< Minor version number
#define YOGI_HDR_VERSION_PATCH 3  ///< Patch version number

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

//! Whole version number of the library (const char*)
#define YOGI_CONST_VERSION_NUMBER 1

//! Major version number of the library (int)
#define YOGI_CONST_VERSION_MAJOR 2

//! Major version number of the library (int)
#define YOGI_CONST_VERSION_MINOR 3

//! Major version number of the library (int)
#define YOGI_CONST_VERSION_PATCH 4

//! Default network interfaces to use for advertising (const char*)
#define YOGI_CONST_DEFAULT_ADV_INTERFACES 5

//! Default multicast addresses to use for advertising (const char*)
#define YOGI_CONST_DEFAULT_ADV_ADDRESS 6

//! Default port to use for advertising via UDP IPv6 multicasts (int)
#define YOGI_CONST_DEFAULT_ADV_PORT 7

//! Default advertising interval in nanoseconds (long long)
#define YOGI_CONST_DEFAULT_ADV_INTERVAL 8

//! Default connection timeout in nanoseconds (long long)
#define YOGI_CONST_DEFAULT_CONNECTION_TIMEOUT 9

//! Default logging verbosity (int)
#define YOGI_CONST_DEFAULT_LOGGER_VERBOSITY 10

//! Default textual format for timestamps in log entries (const char*)
#define YOGI_CONST_DEFAULT_LOG_TIME_FORMAT 11

//! Default textual format for log entries (const char*)
#define YOGI_CONST_DEFAULT_LOG_FORMAT 12

//! Maximum size of the payload in a message (int)
#define YOGI_CONST_MAX_MESSAGE_PAYLOAD_SIZE 13

//! Default textual format for timestamps (const char*)
#define YOGI_CONST_DEFAULT_TIME_FORMAT 14

//! Default string to denote an infinite duration (const char*)
#define YOGI_CONST_DEFAULT_INF_DURATION_STRING 15

//! Default textual format for duration strings (const char*)
#define YOGI_CONST_DEFAULT_DURATION_FORMAT 16

//! Default string to denote an invalid object handle (const char*)
#define YOGI_CONST_DEFAULT_INVALID_HANDLE_STRING 17

//! Default textual format for strings describing an object (const char*)
#define YOGI_CONST_DEFAULT_OBJECT_FORMAT 18

//! Minimum size of a send queue for a remote branch (int)
#define YOGI_CONST_MIN_TX_QUEUE_SIZE 19

//! Maximum size of a send queue for a remote branch (int)
#define YOGI_CONST_MAX_TX_QUEUE_SIZE 20

//! Default size of a send queue for a remote branch (int)
#define YOGI_CONST_DEFAULT_TX_QUEUE_SIZE 21

// Minimum size of a receive queue for a remote branch (int)
#define YOGI_CONST_MIN_RX_QUEUE_SIZE 22

// Maximum size of a receive queue for a remote branch (int)
#define YOGI_CONST_MAX_RX_QUEUE_SIZE 23

// Default size of a receive queue for a remote branch (int)
#define YOGI_CONST_DEFAULT_RX_QUEUE_SIZE 24

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

//! Could not open file
#define YOGI_ERR_OPEN_FILE_FAILED -17

//! Could not read from or write to socket
#define YOGI_ERR_RW_SOCKET_FAILED -18

//! Could not connect a socket
#define YOGI_ERR_CONNECT_SOCKET_FAILED -19

//! The magic prefix sent when establishing a connection is wrong
#define YOGI_ERR_INVALID_MAGIC_PREFIX -20

//! The local and remote branches use incompatible Yogi versions
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
#define YOGI_ERR_WRITE_TO_FILE_FAILED -35

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

//! @}
//!
//! @defgroup VB Log verbosity/severity
//!
//! Verbosity/severity levels for logging.
//!
//! @{

//! Used to disable logging
#define YOGI_VB_NONE -1

//! Fatal errors are error that requires a process restart
#define YOGI_VB_FATAL 0

//! Errors that the system can recover from
#define YOGI_VB_ERROR 1

//! Warnings
#define YOGI_VB_WARNING 2

//! Useful general information about the system state
#define YOGI_VB_INFO 3

//! Information for debugging
#define YOGI_VB_DEBUG 4

//! Detailed debugging
#define YOGI_VB_TRACE 5

//! @}
//!
//! @defgroup BC Boolean Constants
//!
//! Definitions for true and false.
//!
//! @{

//! Represents a logical false
#define YOGI_FALSE 0

//! Represents a logical true
#define YOGI_TRUE 1

//! @}
//!
//! @defgroup ST Stream Constants
//!
//! Constants for differentiating between stdout and stderr.
//!
//! @{

//! Standard output
#define YOGI_ST_STDOUT 0

//! Standard error output
#define YOGI_ST_STDERR 1

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

//! No signal
#define YOGI_SIG_NONE 0

//! Interrupt (e.g. by receiving SIGINT or pressing STRG + C)
#define YOGI_SIG_INT (1 << 0)

//! Termination request (e.g. by receiving SIGTERM)
#define YOGI_SIG_TERM (1 << 1)

//! User-defined signal 1
#define YOGI_SIG_USR1 (1 << 24)

//! User-defined signal 2
#define YOGI_SIG_USR2 (1 << 25)

//! User-defined signal 3
#define YOGI_SIG_USR3 (1 << 26)

//! User-defined signal 4
#define YOGI_SIG_USR4 (1 << 27)

//! User-defined signal 5
#define YOGI_SIG_USR5 (1 << 28)

//! User-defined signal 6
#define YOGI_SIG_USR6 (1 << 29)

//! User-defined signal 7
#define YOGI_SIG_USR7 (1 << 30)

//! User-defined signal 8
#define YOGI_SIG_USR8 (1 << 31)

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

//! Data is encoded as JSON
#define YOGI_ENC_JSON 0

//! Data is encoded as MessagePack
#define YOGI_ENC_MSGPACK 1

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
 * The branch is configured via the \p props parameter. The supplied JSON must
 * have the following structure:
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
 * \param[in]  props    Branch properties as JSON (set to NULL to use defaults)
 * \param[in]  section  Section in \p props to use (set to NULL for root);
 *                      syntax is JSON pointer (RFC 6901)
 * \param[out] err      Pointer to a char array for storing an error description
 *                      (can be set to NULL)
 * \param[in]  errsize  Maximum number of bytes to write to \p err
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_BranchCreate(void** branch, void* context, const char* props,
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
 * Creates a new terminal.
 *
 * Terminals are the communication endpoints in Yogi. Each terminal is
 * associated with exactly one branch and each branch can have an arbitrary
 * number of terminals. Terminals are uniquely identified by their UUID as well
 * as by the combination their path and the branch they belong to.
 *
 * Upon creation, a terminal is either a *prividing* or a *consuming* terminal.
 * Providing terminals have paths (names) that describe where in the owning
 * branch they are located. Consuming terminals have paths that describe where
 * in the network their providing terminal resides. This means that a connection
 * between terminals is always initiated from the consuming terminal; i.e. only
 * the consuming terminal needs to know the location of their connected
 * counterpart.
 *
 * See \ref termtypes for a description of the different terminal types along
 * with their properties as configured via the \p props parameter.
 *
 * \note
 *   The final path of the terminal will be determined by joining the \p pathpfx
 *   and the value of the _path_ string from the \p props JSON. At most one of
 *   these two may be empty or NULL.
 *
 * \param[out] terminal Pointer to the terminal handle
 * \param[in]  branch   The branch to use
 * \param[in]  pathpfx  Path prefix of the terminal (can be NULL)
 * \param[in]  rolesrc  Provider-consumer role source (see \ref RLS)
 * \param[in]  props    Terminal properties as JSON
 * \param[in]  section  Section in \p props to use (NULL means the root
 *                      section; syntax is JSON pointer (RFC 6901))
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 */
YOGI_API int YOGI_TerminalCreate(void** terminal, void* branch,
                                 const char* pathpfx, int rolesrc,
                                 const char* props, const char* section);

// YOGI_API int YOGI_TerminalSend(void* terminal, int msgid, const void* data);

// YOGI_API int YOGI_TerminalReceive(void* terminal, int* msgid, void* data,
//                                   int datasize);

// YOGI_API int YOGI_InterfaceCreate(void** iface, void* branch,
//                                   const char* pathpfx, int role,
//                                   const char* props, const char* section);

// YOGI_API int YOGI_InterfaceGetTerminal(void** terminal, void* iface,
//                                        const char* path);

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

//! @}

#ifdef __cplusplus
}  // extern "C"
#endif

/**
 * \page termtypes Terminal Types
 * \brief Description of the different terminal types.
 *
 * Terminals are the communication endpoints in Yogi. Each terminal is
 * associated with exactly one branch and each branch can have an arbitrary
 * number of terminals. Terminals are uniquely identified by their UUID as well
 * as by the combination their path and the branch they belong to.
 *
 * Upon creation, a terminal is either a *prividing* or a *consuming* terminal.
 * Providing terminals have paths (names) that describe where in the owning
 * branch they are located. Consuming terminals have paths that describe where
 * in the network their providing terminal resides. This means that a connection
 * between terminals is always initiated from the consuming terminal; i.e. only
 * the consuming terminal needs to know the location of their connected
 * counterpart.
 *
 * The format of the terminal properties can vary depending on the type of
 * terminal. However, the basic format is shown in the following example:
 * \code
 *   {
 *     "path":        "Temperature",
 *     "type":        "Variable",
 *     "role":        "Provider",
 *     "description": "The engine's temperature",
 *     "data": [
 *       {
 *         "type":        "float",
 *         "description": "Temperature in degrees Celsius",
 *         "min":         -50,
 *         "max":         300
 *       }, {
 *         "type":        "int",
 *         "description": "Timestamp"
 *       }
 *     ]
 *   }
 * \endcode
 *
 * The properties have the following meaning:
 *  - __path__: Path of the terminal relative its branch. This property is
 *              optional when creating the terminal if the _pathpfx_ parameter
 *              is set when calling YOGI_TerminalCreate().
 *  - __type__: Type of the terminal.
 *  - __role__: Role of the terminal (_Provider_ or _Consumer_). This property
 *              is optional when creating the terminal if the _rolesrc_
 *              parameter is set appropriately when calling
 *              YOGI_TerminalCreate().
 *  - __description__: Additional information about the terminal. This property
 *                     is always optional.
 *  - __data__: Schema for the data type that is transmitted. This section
 *              describes the format of the payload that gets sent over the
 *              terminals (see \ref dataschema for a detailed description).
 *              Instead of a _data_ section, some terminal types require both a
 *              _request_ and _response_ section for defining the payload format
 *              for the different messages.
 *
 *
 * \section tt_variable Variable
 *
 * A single providing _Variable_ terminal "owns" the variable; it is the only
 * terminal that notifies consumers about value changes. The value of the
 * variable is the message that has last been sent over the provider or
 * received by the provider from any of the consuming _Variable_ terminals. Once
 * the value of the variable changes, the provider sends the new value to all
 * consumers. The value gets cached such that new consumers will receive the
 * current value of the variable as soon as possible.
 *
 * This type of terminal is intended to be used in place of normal variables
 * that should be both readable and writable by other processes.
 *
 * Terminal properties:
 * \code
 *   {
 *     "path":        "Temperature",
 *     "type":        "Variable",
 *     "role":        "Provider",
 *     "description": "The engine's temperature",
 *     "data": [
 *       {
 *         "type":        "float",
 *         "description": "Temperature in degrees Celsius",
 *         "min":         -50,
 *         "max":         300
 *       }, {
 *         "type":        "int",
 *         "description": "Timestamp"
 *       }
 *     ],
 *     "initial_value": [
 *       25.0,
 *       0
 *     ]
 *   }
 * \endcode
 *
 * The __initial_value__ property is optional and sets the initial value of the
 * _Variable_. If the value is set on a provider terminal, all consumers will be
 * notified; however, if the value is set on a consumer terminal then only that
 * consumer terminal will see this value and it will be overridden by the next
 * received message from the provider.
 *
 * Compatible terminal types:
 *  - \ref tt_variable as provider or consumer.
 *  - \ref tt_ro_variable as consumer.
 *
 *
 * \section tt_ro_variable Read-Only Variable
 *
 * Same as \ref tt_variable but consumers cannot change the value of the
 * variable.
 *
 * Terminal properties:
 * \code
 *   {
 *     "path":        "RPM",
 *     "type":        "ReadOnlyVariable",
 *     "role":        "Consumer",
 *     "description": "The engine's measured RPM value",
 *     "data": [
 *       {
 *         "type": "int"
 *       }
 *     ],
 *     "initial_value": [
 *       0
 *     ]
 *   }
 * \endcode
 *
 * The __initial_value__ property is optional and sets the initial value of the
 * _Read-Only Variable_. If the value is set on a provider terminal, all
 * consumers will be notified; however, if the value is set on a consumer
 * terminal then only that consumer terminal will see this value and it will be
 * overridden by the next received message from the provider.
 *
 * Compatible terminal types:
 *  - \ref tt_ro_variable as provider or consumer.
 *  - \ref tt_variable as provider.
 *  - \ref tt_const as provider.
 *
 *
 * \section tt_wo_variable Write-Only Variable
 *
 * Same as \ref tt_variable but consumers cannot read the value of the variable.
 *
 * This terminal type is intended to be used as an optimization if consumers do
 * not need to read the state of a _Variable_ and therefore, the provider does
 * not have to notify the consumer about value changes.
 *
 * Terminal properties:
 * \code
 *   {
 *     "path":        "Logout Time",
 *     "type":        "WriteOnlyVariable",
 *     "role":        "Provider",
 *     "description": "Time when the user logged out",
 *     "data": [
 *       {
 *         "type":        "int",
 *         "description": "Timestamp"
 *       }
 *     ],
 *     "initial_value": [
 *       0
 *     ]
 *   }
 * \endcode
 *
 * The __initial_value__ property is optional and only valid for providers. It
 * sets the initial value of the _Write-Only Variable_. The property will be
 * ignored for consumers.
 *
 * Compatible terminal types:
 *  - \ref tt_wo_variable as provider or consumer.
 *  - \ref tt_variable as provider.
 *
 *
 * \section tt_const Constant
 *
 * Same as \ref tt_ro_variable but the value of the variable never changes after
 * it has initially been set. This type of terminal is useful for making parts
 * of a process' configuration available to other processes.
 *
 * Terminal properties:
 * \code
 *   {
 *     "path":        "Debug Mode",
 *     "type":        "Constant",
 *     "role":        "Provider",
 *     "description": "True if debug mode is active",
 *     "data": [
 *       {
 *         "type": "bool"
 *       }
 *     ],
 *     "value": [
 *       false
 *     ]
 *   }
 * \endcode
 *
 * The __value__ property is optional and only valid for providers. It sets the
 * value of the _Constant_. The property will be ignored for consumers.
 *
 * \attention
 *   Setting the __value__ property has the same effect as initially sending
 *   the value of the _Constant_ manually, i.e. if the property is set, any
 *   further attempts to set the value will fail.
 *
 * Compatible terminal types:
 *  - \ref tt_const as provider or consumer.
 *  - \ref tt_ro_variable as consumer.
 *
 *
 * \section tt_publisher Publisher
 *
 * Sends messages to an arbitrary number of \ref tt_subscriber terminals.
 *
 * Compatible terminal types:
 *  - \ref tt_subscriber as provider or consumer.
 *
 *
 * \section tt_subscriber Subscriber
 *
 * Receives messages published by \ref tt_publisher terminals.
 *
 * Compatible terminal types:
 *  - \ref tt_publisher as provider or consumer.
 *
 *
 * \section tt_surveyor Surveyor
 *
 * Sends a request message to an arbitrary number of \ref tt_surveyee terminals
 * that in turn answer with a response message.
 *
 * Compatible terminal types:
 *  - \ref tt_surveyee as provider or consumer.
 *
 *
 * \section tt_surveyee Surveyee
 *
 * Answers messages from \ref tt_surveyor terminals by sending a response
 * message for each received request.
 *
 * Compatible terminal types:
 *  - \ref tt_surveyor as provider or consumer.
 *
 *
 * \section tt_pair Pair
 *
 * Allows messaging between exactly one providing and one consuming terminal.
 *
 * Compatible terminal types:
 *  - \ref tt_pair as provider or consumer.
 *
 *
 * \section tt_semaphore Semaphore
 *
 * Can be acquired via \ref tt_lock terminals a configured number of N times
 * at any one time.
 *
 * Compatible terminal types:
 *  - \ref tt_lock as provider or consumer.
 *
 *
 * \section tt_mutex Mutex
 *
 * Special case of a \ref tt_semaphore where N = 1, i.e. it can only be acquired
 * by a single \ref tt_lock terminal at any one time.
 *
 * Compatible terminal types:
 *  - \ref tt_lock as provider or consumer.
 *
 *
 * \section tt_lock Lock
 *
 * Acquires \ref tt_semaphore and \ref tt_mutex terminals.
 *
 * Compatible terminal types:
 *  - \ref tt_semaphore as provider or consumer.
 *  - \ref tt_mutex as provider or consumer.
 *
 *
 * \section tt_stream Stream
 *
 * Allows streaming of data, e.g. for video or audio data. The provider
 * represents the source and the consumers represent an arbitrary number of
 * sinks of the stream.
 *
 * Compatible terminal types:
 *  - \ref tt_stream as provider or consumer.
 *
 *
 * \section tt_file File
 *
 * Allows transferring files. The provider supplies the file to an arbitrary
 * number of consumers.
 *
 * Compatible terminal types:
 *  - \ref tt_file as provider or consumer.
 */

/**
 * \page dataschema Data Schemas
 * \brief Description of the syntax and semantics of data schemas.
 *
 * TBD
 */

#endif  // YOGI_CORE_H
