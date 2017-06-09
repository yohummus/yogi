#ifndef YOGI_CORE_H
#define YOGI_CORE_H

//! @defgroup ERRORCODES Error codes
//!
//! YOGI error codes are always < 0 and a human-readable description of an
//! error code can be obtained by calling YOGI_GetErrorString().
//!
//! @{

//! The operation completed successfully
#define YOGI_OK 0

//! Unknown internal error occured
#define YOGI_ERR_UNKNOWN -1

//! Invalid object handle
#define YOGI_ERR_INVALID_HANDLE -2

//! Object is of the wrong type
#define YOGI_ERR_WRONG_OBJECT_TYPE -3

//! Object is still used
#define YOGI_ERR_OBJECT_STILL_USED -4

//! Insufficient memory to complete the operation
#define YOGI_ERR_BAD_ALLOCATION -5

//! Invalid parameter
#define YOGI_ERR_INVALID_PARAM -6

//! Already connected
#define YOGI_ERR_ALREADY_CONNECTED -7

//! There is already an object with the same identifier
#define YOGI_ERR_AMBIGUOUS_IDENTIFIER -8

//! The library has already been initialised
#define YOGI_ERR_ALREADY_INITIALISED -9

//! The library has not been initialised
#define YOGI_ERR_NOT_INITIALISED -10

//! Could not create the log file
#define YOGI_ERR_CANNOT_CREATE_LOG_FILE -11

//! The operation has been canceled
#define YOGI_ERR_CANCELED -12

//! An asynchronous operation has already been started
#define YOGI_ERR_ASYNC_OPERATION_RUNNING -13

//! The provided buffer is not big enough
#define YOGI_ERR_BUFFER_TOO_SMALL -14

//! No remote Terminals are currently bound to the local Terminal
#define YOGI_ERR_NOT_BOUND -15

//! Invalid ID
#define YOGI_ERR_INVALID_ID -16

//! Identification data is too large
#define YOGI_ERR_IDENTIFICATION_TOO_LARGE -17

//! Invalid IP address
#define YOGI_ERR_INVALID_IP_ADDRESS -18

//! Invalid port number
#define YOGI_ERR_INVALID_PORT_NUMBER -19

//! Could not open socket
#define YOGI_ERR_CANNOT_OPEN_SOCKET -20

//! Could not bind socket
#define YOGI_ERR_CANNOT_BIND_SOCKET -21

//! Could not listen on socket
#define YOGI_ERR_CANNOT_LISTEN_ON_SOCKET -22

//! Could not read from or write to socket
#define YOGI_ERR_SOCKET_BROKEN -23

//! Received an invalid magic prefix
#define YOGI_ERR_INVALID_MAGIC_PREFIX -24

//! Incompatible version
#define YOGI_ERR_INCOMPATIBLE_VERSION -25

//! Failed to accept incoming connection
#define YOGI_ERR_ACCEPT_FAILED -26

//! Timeout
#define YOGI_ERR_TIMEOUT -27

//! Address is already in use
#define YOGI_ERR_ADDRESS_IN_USE -28

//! Resolving IP address failed
#define YOGI_ERR_RESOLVE_FAILED -29

//! Connection refused
#define YOGI_ERR_CONNECTION_REFUSED -30

//! Host is unreachable
#define YOGI_ERR_HOST_UNREACHABLE -31

//! Network is down
#define YOGI_ERR_NETWORK_DOWN -32

//! Could not establish connection
#define YOGI_ERR_CONNECT_FAILED -33

//! The operation is not ready to be performed
#define YOGI_ERR_NOT_READY -34

//! The connection has already been assigned
#define YOGI_ERR_ALREADY_ASSIGNED -35

//! The connection is not alive any more
#define YOGI_ERR_CONNECTION_DEAD -36

//! The connection has been closed gracefully by the remote host
#define YOGI_ERR_CONNECTION_CLOSED -37

//! Not yet initialized
#define YOGI_ERR_UNINITIALIZED -38

//! @}
//!
//! @defgroup VERBOSITY Log verbosity
//!
//! Verbosity of the log files
//!
//! @{

//! Only log fatal errors.
#define YOGI_VB_FATAL 0

//! Log fatal and normal errors.
#define YOGI_VB_ERROR 1

//! Log errors and warnings.
#define YOGI_VB_WARNING 2

//! Log errors, warnings and additional information.
#define YOGI_VB_INFO 3

//! Log errors, warnings, additional information and debug messages.
#define YOGI_VB_DEBUG 4

//! Maximum log level; contains information for detailed debugging.
#define YOGI_VB_TRACE 5

//! @}
//!
//! @defgroup TERMTYPES Terminal types
//!
//! Description of possible Terminal types.
//!
//! @{

//! Deaf-Mute Terminal.
//!
//! Those Terminals do not exchange data at all. They are useful when a process
//! only needs to detect the presence of a Terminal.
#define YOGI_TM_DEAFMUTE 0

//! Publish-Subscribe Terminal.
//!
//! As their name suggests, Publish-Subscribe Terminals follow the
//! publish-subscribe messaging pattern, i.e. a message published via a Terminal
//! T will be sent to all remote Terminals which are bound to T. This Terminal
//! type is a low-level building block for more complex types and is rarely used
//! on its own.
#define YOGI_TM_PUBLISHSUBSCRIBE 1

//! Scatter-Gather Terminal.
//!
//! Scatter-Gather Terminals implement Remote Procedure Calls (RPCs). A
//! scatter-gather operation consists of two phases, the scatter phase and the
//! gather phase. During the scatter phase, the initiating Terminal sends a
//! message to all bound remote Terminals (using the publish-subscribe pattern).
//! During the gather phase, the initiating Terminal waits for responses from
//! all remote Terminals that it sent the message to. Once all those responses
//! have been received, the scatter-gather operation has completed. Also a
//! low-level building block.
#define YOGI_TM_SCATTERGATHER 2

//! Cached Publish-Subscribe Terminal.
//!
//! Same behaviour as Publish-Subscribe Terminals with the addition of a cache
//! which always contains the last message a Terminal received. This is useful
//! for distributing values that do not change very often. This Terminal type is
//! also a low-level building block.
#define YOGI_TM_CACHEDPUBLISHSUBSCRIBE 3

//! Producer Terminal.
//!
//! A Producer Terminal only publishes messages while a Consumer Terminal only
//! receives messages. Consumer Terminals automatically bind to Producer
//! Terminals with the same name and the publish-subscribe pattern is used for
//! messaging.
#define YOGI_TM_PRODUCER 4

//! Consumer Terminal.
//!
//! A Producer Terminal only publishes messages while a Consumer Terminal only
//! receives messages. Consumer Terminals automatically bind to Producer
//! Terminals with the same name and the publish-subscribe pattern is used for
//! messaging.
#define YOGI_TM_CONSUMER 5

//! Cached Producer Terminal.
//!
//! Same as Producer-Consumer Terminals but with a cache containing the most
//! recent message the Terminal received. Useful for distributing values that do
//! not change very often.
#define YOGI_TM_CACHEDPRODUCER 6

//! Cached Consumer Terminal.
//!
//! Same as Producer-Consumer Terminals but with a cache containing the most
//! recent message the Terminal received. Useful for distributing values that do
//! not change very often.
#define YOGI_TM_CACHEDCONSUMER 7

//! Master Terminal.
//!
//! Master Terminals send messages of type A to all bound Slave Terminals and
//! Slave Terminals send messages of type B to all bound Master Terminals. Both
//! Master and Slave Terminals automatically bind to their counterpart with the
//! same name. Use Master Terminals for the "owner" or source of the data and
//! Slave Terminals for users of the data.
#define YOGI_TM_MASTER 8

//! Slave Terminal.
//!
//! Master Terminals send messages of type A to all bound Slave Terminals and
//! Slave Terminals send messages of type B to all bound Master Terminals. Both
//! Master and Slave Terminals automatically bind to their counterpart with the
//! same name. Use Master Terminals for the "owner" or source of the data and
//! Slave Terminals for users of the data.
#define YOGI_TM_SLAVE 9

//! Cached Master Terminal.
//!
//! Same as Master-Slave Terminals but with a cache containing the most recent
//! message the Terminal received. Useful for distributing values that do not
//! change very often.
#define YOGI_TM_CACHEDMASTER 10

//! Cached Slave Terminal.
//!
//! Same as Master-Slave Terminals but with a cache containing the most recent
//! message the Terminal received. Useful for distributing values that do not
//! change very often.
#define YOGI_TM_CACHEDSLAVE 11

//! Service Terminal.
//!
//! These Terminals behave like Scatter-Gather Terminals whereby only the Client
//! Terminals can send requests and only the Service Terminals can respond to
//! those requests.
#define YOGI_TM_SERVICE 12

//! Client Terminal.
//!
//! These Terminals behave like Scatter-Gather Terminals whereby only the Client
//! Terminals can send requests and only the Service Terminals can respond to
//! those requests.
#define YOGI_TM_CLIENT 13

//! @}
//!
//! @defgroup SCATGATHFLAGS Flags for Scatter-Gather operations
//!
//! Description of Scatter-Gather operation flags.
//!
//! @{

//! No flags set.
#define YOGI_SG_NOFLAGS 0

//! The scatter-gather operation has finished.
#define YOGI_SG_FINISHED (1<<0)

//! The remote Terminal has actively ignored the scattered message.
#define YOGI_SG_IGNORED (1<<1)

//! The remote Terminal has not been listening for scattered messages when it
//! received the scattered message.
#define YOGI_SG_DEAF (1<<2)

//! The remote Binding has been destroyed.
#define YOGI_SG_BINDINGDESTROYED (1<<3)

//! The connection between the local and the remote Leaf has been lost.
#define YOGI_SG_CONNECTIONLOST (1<<4)

//! @}
//!
//! @defgroup CTRLFLOW Control flow commands
//!
//! Control flow commands returned from handler functions.
//!
//! @{

#define YOGI_DO_CONTINUE 0
#define YOGI_DO_STOP 1

//! @}
//!
//! @defgroup BINDSTATES Binding states
//!
//! Description of possible states of a Binding.
//!
//! @{

//! The Binding is released
#define YOGI_BD_RELEASED 0

//! The Binding is established
#define YOGI_BD_ESTABLISHED 1

//! @}
//!
//! @defgroup SUBSTATES Subscription states
//!
//! Description of possible states of a subscription.
//!
//! @{

//! Not subscribed
#define YOGI_SB_UNSUBSCRIBED 0

//! Subscribed
#define YOGI_SB_SUBSCRIBED 1

//! @}

#ifndef YOGI_API
#   ifdef _MSC_VER
#       define YOGI_API __declspec(dllimport)
#   else
#       define YOGI_API
#   endif
#endif

//! @defgroup FUNCTIONS Library functions
//!
//! Description of the library functions
//!
//! @{

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Obtains the version of the library.
 *
 * @returns Library version string
 ******************************************************************************/
YOGI_API const char* YOGI_GetVersion();

/***************************************************************************//**
 * Obtains a description of an error code.
 *
 * Use this function to obtain a human readable description of an error code.
 * The function will always return a valid pointer, even if the error code is
 * not valid. In this case, the description will tell you that.
 *
 * @param[in] errCode The error code (see \ref ERRORCODES)
 *
 * @returns Description of the error code
 ******************************************************************************/
YOGI_API const char* YOGI_GetErrorString(int errCode);

/***************************************************************************//**
 * Sets the path and verbosity for the log file.
 *
 * By default, logging is disabled. This function sets up the logger to write
 * entries with a severity of at least \p verbosity to the specified \p file. If
 * \p file is set to NULL, logging will be disabled.
 *
 * This function can be called before the library has been initialized and after
 * the library has been shut down.
 *
 * @param[in] file      Path to the log file (NULL disables logging)
 * @param[in] verbosity The verbosity of the log file (see \ref VERBOSITY)
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SetLogFile(const char* file, int verbosity);

/***************************************************************************//**
 * Initialises the DLL.
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_Initialise();

/***************************************************************************//**
 * Cancels all active asyncronous operations, destroys all created objects and
 * releases all resources.
 *
 * Before an object is destroyed, any active asynchronous operations will be
 * canceled and the completion handlers will be called with an error code of
 * #YOGI_ERR_CANCELED.
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_Shutdown();

/***************************************************************************//**
 * Destroys an object.
 *
 * Tries to destroy the object with the given \p handle. If any other object
 * depends on this object, destroying it will fail and
 * #YOGI_ERR_OBJECT_STILL_USED will be returned.
 *
 * Destroying an object will cause any active asynchronous operations to get
 * canceled and the corresponding completion handlers will be invoked with an
 * error code of #YOGI_ERR_CANCELED.
 *
 * @param[in] object Handle of the object
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_Destroy(void* object);

/***************************************************************************//**
 * Creates a new scheduler.
 *
 * A scheduler manages a thread pool with an arbitrary number of threads. After
 * construction, the pool consists of only a single thread, but the size of the
 * pool can be adjusted at any time via YOGI_SetSchedulerThreadPoolSize().
 *
 * The parameter \p scheduler is only set if the operation succeeds. If an error
 * occurs, the parameter will not be altered.
 *
 * @param[out] scheduler Pointer to the scheduler handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateScheduler(void** scheduler);

/***************************************************************************//**
 * Sets the number of threads in a scheduler's thread pool.
 *
 * @param[in] scheduler  Scheduler handle
 * @param[in] numThreads Number of threads
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SetSchedulerThreadPoolSize(void* scheduler,
    unsigned numThreads);

/***************************************************************************//**
 * Creates a Node.
 *
 * A Node interconnects Leafs and other Nodes. Nodes support an arbitrary number
 * of connections and can be seen as hubs/switches in the virtual network.
 *
 * @param[out] node      Pointer to the Node handle
 * @param[in]  scheduler Scheduler handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateNode(void** node, void* scheduler);

/***************************************************************************//**
 * Gets descriptions of all terminals known to a node
 *
 * The terminal descriptions are written to \p buffer one after another, with a
 * structure of each description as follows:
 *  -# Byte 0: Type of the terminal (see \ref TERMTYPES)
 *  -# Bytes 1-4: Signature of the terminal
 *  -# Bytes 5-N: Name of the terminal (NULL-terminated)
 *
 * @param[in]  node         Node handler
 * @param[out] buffer       Buffer to copy the information to
 * @param[in]  bufferSize   Size of the buffer
 * @param[out] numTerminals Number terminal descriptions written to \p buffer
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_GetKnownTerminals(void* node, void* buffer,
    unsigned bufferSize, unsigned* numTerminals);

/***************************************************************************//**
 * Asynchronously waits for a change of the terminals known to a node
 *
 * The structure of the data written to \p buffer is as follows:
 *  -# Byte 0: 0 = terminal removed; 1 = terminal added
 *  -# Byte 1: Type of the terminal (see \ref TERMTYPES)
 *  -# Bytes 2-5: Signature of the terminal
 *  -# Bytes 6-N: Name of the terminal (NULL-terminated)
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  node       Node handler
 * @param[out] buffer     Buffer to copy the information to
 * @param[in]  bufferSize Size of the buffer
 * @param[in]  handlerFn  Completion handler
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncAwaitKnownTerminalsChange(void* node, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, void*), void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous operation started via
 * YOGI_AsyncAwaitKnownTerminalsChange().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] node Node handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CancelAwaitKnownTerminalsChange(void* node);

/***************************************************************************//**
 * Creates a Leaf.
 *
 * A Leaf groups Terminals together and integrates them into the virtual
 * network. Leafs only support a single connection which can either be connected
 * to another Leaf or to a Node.
 *
 * @param[out] leaf      Pointer to the Leaf handle
 * @param[in]  scheduler Scheduler handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateLeaf(void** leaf, void* scheduler);

/***************************************************************************//**
 * Creates a Terminal.
 *
 * The type of the Terminal is determined by the value of the \p type parameter.
 * Multiple Terminals may have the same name if and only if they are of
 * different Terminal types. Possible types are:
 *  - #YOGI_TM_DEAFMUTE
 *  - #YOGI_TM_PUBLISHSUBSCRIBE
 *  - #YOGI_TM_SCATTERGATHER
 *
 * Each Terminal has a certain user-defined signature which allows the library
 * user to further differentiate Terminal types. Only Terminals with the same
 * signature can be bound together.
 *
 * @param[out] terminal  Pointer to the Terminal handle
 * @param[in]  leaf      Handle of the associated Leaf
 * @param[in]  type      Type of the Terminal (see \ref TERMTYPES)
 * @param[in]  name      Name of the Terminal
 * @param[in]  signature User-defined signature of the Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateTerminal(void** terminal, void* leaf, int type,
    const char* name, unsigned signature);

/***************************************************************************//**
 * Queries a Terminal's subscription state.
 *
 * If the operation is successful, the target location of the \p state
 * parameter will be set to the Terminal's subscription state (see \ref
 * SUBSTATES).
 *
 * @param[in]  terminal Terminal handle
 * @param[out] state    Pointer to where the state shall be stored
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_GetSubscriptionState(void* terminal, int* state);

/***************************************************************************//**
 * Asynchronously queries a Terminal's subscription state.
 *
 * This function invokes the completion handler \p handlerFn with the current
 * subscription state.
 *
 * Propagation of the subscription state is blocked during the invokation of
 * \p handlerFn to avoid losing state changes within that time. In order to keep
 * track of future state changes, the YOGI_AsyncAwaitSubscriptionStateChange()
 * function should be called from within \p handlerFn.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Current state of the subscription (see \ref SUBSTATES)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in] terminal  Terminal handle
 * @param[in] handlerFn Completion handler
 * @param[in] userArg   User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncGetSubscriptionState(void* terminal,
    void (*handlerFn)(int, int, void*), void* userArg);

/***************************************************************************//**
 * Asynchronously waits for a subscription's state to change.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * the state of the subscription changes.
 *
 * Propagation of the subscription state is blocked during the invokation of
 * \p handlerFn to avoid losing state changes within that time. In order to keep
 * track of all state changes, this function should be called again within \p
 * handlerFn.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# New state of the subscription (see \ref SUBSTATES)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in] terminal  Terminal handle
 * @param[in] handlerFn Completion handler
 * @param[in] userArg   User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncAwaitSubscriptionStateChange(void* terminal,
    void (*handlerFn)(int, int, void*), void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous operation started via
 * YOGI_AsyncAwaitSubscriptionStateChange().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Terminal handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CancelAwaitSubscriptionStateChange(void* terminal);

/***************************************************************************//**
 * Creates a Binding for a local Terminal to one or more remote Terminals.
 *
 * @param[out] binding  Pointer to the Binding handle
 * @param[in]  terminal Handle of the associated local Terminal
 * @param[in]  targets  Name of the target Terminal(s)
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateBinding(void** binding, void* terminal,
    const char* targets);

/***************************************************************************//**
 * Queries a Binding's state.
 *
 * If the operation is successful, the target location of the \p state
 * parameter will be set to the state of the Binding (see \ref BINDSTATES).
 *
 * @param[in]  object Binding or Terminal handle
 * @param[out] state  Pointer to where the Binding's state shall be stored
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_GetBindingState(void* object, int* state);

/***************************************************************************//**
 * Asynchronously queries a Binding's state.
 *
 * This function invokes the completion handler \p handlerFn with the current
 * state of the Binding. Only one asynchronous operation can be active for a
 * Binding at any time.
 *
 * Propagation of the Binding's state is blocked during the invokation of
 * \p handlerFn to avoid losing state changes within that time. In order to keep
 * track of future state changes, the YOGI_AsyncAwaitBindingStateChange()
 * function should be called from within \p handlerFn.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Current state of the Binding (see \ref BINDSTATES)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in] binding   Binding handle
 * @param[in] handlerFn Completion handler
 * @param[in] userArg   User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncGetBindingState(void* binding,
    void (*handlerFn)(int, int, void*), void* userArg);

/***************************************************************************//**
 * Asynchronously waits for a Binding's state to change.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * the state of the \p binding changes. Only one asynchronous operation can be
 * active for a Binding at any time.
 *
 * Propagation of the Binding's state is blocked during the invokation of
 * \p handlerFn to avoid losing state changes within that time. In order to keep
 * track of all state changes, this function should be called again within \p
 * handlerFn.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# New state of the Binding (see \ref BINDSTATES)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in] binding   Binding handle
 * @param[in] handlerFn Completion handler
 * @param[in] userArg   User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncAwaitBindingStateChange(void* binding,
    void (*handlerFn)(int, int, void*), void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous operation started via
 * YOGI_AsyncAwaitBindingStateChange().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] binding Binding handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CancelAwaitBindingStateChange(void* binding);

/***************************************************************************//**
 * Creates a connection between local Leafs/Nodes.
 *
 * An internal connection should be used to interconnect two locally created
 * Leafs and Nodes. The connection can be established between two Nodes, two
 * Leafs or one of each.
 *
 * Local connections are not monitored via heartbeats.
 *
 * @param[out] connection Pointer to the connection handle
 * @param[in]  leafNodeA  Handle of the 1st Leaf/Node
 * @param[in]  leafNodeB  Handle of the 2nd Leaf/Node
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateLocalConnection(void** connection, void* leafNodeA,
    void* leafNodeB);

/***************************************************************************//**
 * Creates a TCP server
 *
 * A TCP server listen for incoming connections from TCP clients. Before
 * communication begins, the server and client exchange identification data
 * which allows for basic authentication mechanisms. Either one of them can
 * deny a connection request based on the received identification data.
 *
 * @param[out] tcpServer Pointer to the server handle
 * @param[in]  scheduler Scheduler handle
 * @param[in]  address   IPv4/IPv6 address to listen on
 * @param[in]  port      TCP port to listen on
 * @param[in]  ident     Identification data (or NULL)
 * @param[in]  identSize Size of the identification data in bytes
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateTcpServer(void** tcpServer, void* scheduler,
	const char* address, unsigned port, const void* ident, unsigned identSize);

/***************************************************************************//**
 * Asynchronously waits for an incoming TCP connection request
 *
 * A pending connection request has to be accepted by assigning the connection
 * to either a node or a leaf or denied by destroying the connection.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Handler of the created TCP connection
 *  -# User-defined parameter \p userArg
 *
 * @param[in] tcpServer Server handle
 * @param[in] hsTimeout Handshake timeout in milliseconds (-1 for infinity)
 * @param[in] handlerFn Completion handler
 * @param[in] userArg   User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncTcpAccept(void* tcpServer, int hsTimeout,
	void (*handlerFn)(int, void*, void*), void* userArg);

/***************************************************************************//**
 * Cancels an asynchronous accept operation
 *
 * @param[in] tcpServer Server handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CancelTcpAccept(void* tcpServer);

/***************************************************************************//**
 * Creates a TCP client
 *
 * A TCP client connects to a remote TCP server. Before communication begins,
 * the server and client exchange identification data which allows for basic
 * authentication mechanisms. Either one of them can deny a connection request
 * based on the received identification data.
 *
 * @param[out] tcpClient Pointer to the client handle
 * @param[in]  scheduler Scheduler handle
 * @param[in]  ident     Identification data (or NULL)
 * @param[in]  identSize Size of the identification data in bytes
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CreateTcpClient(void** tcpClient, void* scheduler,
	const void* ident, unsigned identSize);

/***************************************************************************//**
 * Asynchronously connects to a TCP server
 *
 * A pending connection request has to be accepted by assigning the connection
 * to either a node or a leaf or denied by destroying the connection.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Handler of the created TCP connection
 *  -# User-defined parameter \p userArg
 *
 * @param[in] tcpClient Client handle
 * @param[in] host      Hostname or IPv4/IPv6 address of the remote host
 * @param[in] port      TCP port of the remote host
 * @param[in] hsTimeout Handshake timeout in milliseconds (-1 for infinity)
 * @param[in] handlerFn Completion handler
 * @param[in] userArg   User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncTcpConnect(void* tcpClient, const char* host,
    unsigned port, int hsTimeout, void (*handlerFn)(int, void*, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an asynchronous connect operation
 *
 * @param[in] tcpClient Client handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CancelTcpConnect(void* tcpClient);

/***************************************************************************//**
 * Gets a human-readable description of a connection
 *
 * This function copies up to \p bufferSize - 1 bytes into \p buffer. The string
 * \p buffer will always be null-terminated.
 *
 * @param[in] connection Connection handle
 * @param[in] buffer     Buffer to copy the description string into
 * @param[in] bufferSize Size of the buffer
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_GetConnectionDescription(void* connection, char* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Gets the YOGI version that the remote end uses
 *
 * This function copies up to \p bufferSize - 1 bytes into \p buffer. The string
 * \p buffer will always be null-terminated.
 *
 * @param[in] connection Connection handle
 * @param[in] buffer     Buffer to copy the version string into
 * @param[in] bufferSize Size of the buffer
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_GetRemoteVersion(void* connection, char* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Gets the identification received from the remote end
 *
 * @param[in]  connection Connection handle
 * @param[out] buffer     Buffer to copy the identification data into
 * @param[in]  bufferSize Size of the buffer
 * @param[out] size       Size of the identification in bytes (may be NULL)
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_GetRemoteIdentification(void* connection, void* buffer,
    unsigned bufferSize, unsigned* size);

/***************************************************************************//**
 * Assigns a connection to a node or leaf
 *
 * A connection can only be assigned once.
 *
 * @param[in] connection Connection handle
 * @param[in] leafNode   Handle of the target leaf or node
 * @param[in] timeout    Timeout of the connection in milliseconds
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AssignConnection(void* connection, void* leafNode,
    int timeout);

/***************************************************************************//**
 * Asynchronously waits for a connection to die
 *
 * A connection cannot die before it has been assigned to a leaf or node.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# User-defined parameter \p userArg
 *
 * @param[in] connection Connection handle
 * @param[in] handlerFn  Completion handler
 * @param[in] userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_AsyncAwaitConnectionDeath(void* connection,
	void (*handlerFn)(int, void*), void* userArg);

/***************************************************************************//**
 * Cancels an asynchronous await death operation
 *
 * @param[in] connection Connection handle
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CancelAwaitConnectionDeath(void* connection);

/***************************************************************************//**
 * Publishes a message on a Publish-Subscribe Terminal.
 *
 * Sends the given data to all remote Terminals which have a Binding to the
 * local Terminal.
 *
 * @param[in] terminal   Handle of the Publish-Subscribe Terminal
 * @param[in] buffer     Pointer to the beginning of the data to send
 * @param[in] bufferSize Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_PS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Asynchronously receives a message published by a remote Publish-Subscribe
 * Terminal.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * a message published by a remote Terminal is received. Messages that are
 * received without an asynchronous receive operation being active are dropped.
 * In order to avoid missing messages, this function should be called again
 * within \p handlerFn.
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, \p handlerFn will be
 * called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p buffer
 * will still be filled with the first \p bufferSize received payload bytes and
 * the size of the received payload parameter will be set to the number of
 * payload bytes in the received message.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  terminal   Handle of the Terminal to receive the message from
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of \p buffer in bytes
 * @param[in]  handlerFn  Function to call when the message has been received
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_PS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_PS_AsyncReceiveMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Publish-Subscribe Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_PS_CancelReceiveMessage(void* terminal);

/***************************************************************************//**
 * Initiates a Scatter-Gather operation and asynchronously waits for responses
 * from remote Terminals which have a Binding to the scattering Terminal.
 *
 * After having sent the data to the remote Terminals which have a Binding to
 * the initiating Terminal, the Terminal waits for responses from those
 * Terminals. For each response, the completion handler \p handlerFn will be
 * invoked with the data sent back from the corresponding remote Terminal.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Operation ID returned by YOGI_SG_AsyncScatterGather()
 *  -# Flags (see \ref SCATGATHFLAGS)
 *  -# Size of the received payload (can be greater than \p gathSize)
 *  -# User-defined parameter \p userArg
 *
 * On reception of the data from the very last remote Terminal, the
 * #YOGI_SG_FINISHED flag will be set.
 *
 * If the completion handler \p handlerFn returns #YOGI_DO_STOP, the
 * Scatter-Gather operation will be aborted and \p handlerFn will not be called
 * any more, even if the #YOGI_SG_FINISHED flag has not been set. In order to
 * continue to receive data, return #YOGI_DO_CONTINUE.
 *
 * If the operation gets canceled via YOGI_SG_CancelScatterGather(), the
 * \p handlerFn callback function will be called with an error code of
 * #YOGI_ERR_CANCELED and the #YOGI_SG_FINISHED flag set.
 *
 * The target memory location and maximum size for the received message's
 * payload is specified by the parameters \p gathBuf and \p gathSize. If the
 * payload of the received message is larger than \p gathSize, the handler
 * function will be called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL.
 * However, \p gathBuf will still be filled with the first \p gathSize received
 * payload bytes and the size of the received payload parameter will be set to
 * the number of payload bytes in the received message.
 *
 * @param[in]  terminal   Handle of the Scatter-Gather Terminal
 * @param[in]  scatBuf    Pointer to the beginning of the data to send
 * @param[in]  scatSize   Number of bytes to send
 * @param[out] gathBuf    Buffer to write gathered data to
 * @param[in]  gathSize   Size of \p gathBuf in bytes
 * @param[in]  handlerFn  Completion handler
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [>0] ID of the operation if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SG_AsyncScatterGather(void* terminal, const void* scatBuf,
    unsigned scatSize, void* gathBuf, unsigned gathSize,
    int (*handlerFn)(int, int, int, unsigned, void*), void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous Scatter-Gather operation started via
 * YOGI_SG_AsyncScatterGather().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal    Terminal handle
 * @param[in] operationId ID of the Scatter-Gather operation
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SG_CancelScatterGather(void* terminal, int operationId);

/***************************************************************************//**
 * Asynchronously waits for an incoming, scattered message.
 *
 * The completion handler \p handlerFn gets called once the scattered message
 * is received. The user code is responsible for reporting back by calling
 * either YOGI_SG_RespondToScatteredMessage() or YOGI_SG_IgnoreScatteredMessage().
 * If neither of those functions is called, the Scatter-Gather operation on the
 * scattering Terminal will never finish (unless canceled).
 *
 * If no asynchronous receive operation is running while a scattered message is
 * received, the Leaf will automatically report back to the scattering Terminal
 * with the #YOGI_SG_DEAF flag set.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Operation ID
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# User-defined parameter \p userArg
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, the handler function will
 * be called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p
 * buffer will still be filled with the first \p bufferSize received payload
 * bytes and the size of the received payload parameter will be set to the
 * number of payload bytes in the received message.
 *
 * @param[in]  terminal   Handle of the Terminal to receive the message from
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of the buffer in bytes
 * @param[in]  handlerFn  Completion handler
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SG_AsyncReceiveScatteredMessage(void* terminal,
    void* buffer, unsigned bufferSize,
    void (*handlerFn)(int, int, unsigned, void*), void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_SG_AsyncReceiveScatteredMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Scatter-Gather Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SG_CancelReceiveScatteredMessage(void* terminal);

/***************************************************************************//**
 * Responds to a received Scatter-Gather request by sending back data.
 *
 * @param[in] terminal    Handle of the Scatter-Gather Terminal
 * @param[in] operationId ID of the Scatter-Gather operation
 * @param[in] buffer      Pointer to the beginning of the data to send
 * @param[in] bufferSize  Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SG_RespondToScatteredMessage(void* terminal,
    int operationId, const void* buffer, unsigned bufferSize);

/***************************************************************************//**
 * Ignores a received Scatter-Gather request.
 *
 * This reports back with the #YOGI_SG_IGNORED flag set.
 *
 * @param[in] terminal    Handle of the Scatter-Gather Terminal
 * @param[in] operationId ID of the Scatter-Gather operation
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SG_IgnoreScatteredMessage(void* terminal, int operationId);

/***************************************************************************//**
 * Publishes a message on a Cached Publish-Subscribe Terminal.
 *
 * Sends the given data to all remote Terminals which have a Binding to the
 * local Terminal.
 *
 * @param[in] terminal   Handle of the Cached Publish-Subscribe Terminal
 * @param[in] buffer     Pointer to the beginning of the data to send
 * @param[in] bufferSize Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Retrieves the last message published by a remote Cached Publish-Subscribe
 * Terminal.
 *
 * @param[in]  terminal     Handle of the Cached Publish-Subscribe Terminal
 * @param[out] buffer       Buffer to write the payload to
 * @param[in]  bufferSize   Size of \p buffer in bytes
 * @param[out] bytesWritten Number of bytes written to \p buffer
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPS_GetCachedMessage(void* terminal, void* buffer,
    unsigned bufferSize, unsigned* bytesWritten);

/***************************************************************************//**
 * Asynchronously receives a message published by a remote Cached
 * Publish-Subscribe Terminal.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * a message published by a remote Terminal is received. Messages that are
 * received without an asynchronous receive operation being active are dropped.
 * In order to avoid missing messages, this function should be called again
 * within \p handlerFn.
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, \p handlerFn will be
 * called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p buffer
 * will still be filled with the first \p bufferSize received payload bytes and
 * the size of the received payload parameter will be set to the number of
 * payload bytes in the received message.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# 1 if the received message is a cached one; 0 otherwise
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  terminal   Handle of the Terminal to receive the message from
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of \p buffer in bytes
 * @param[in]  handlerFn  Function to call when the message has been received
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, int, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_AsyncReceiveCachePublishedMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Cached Publish-Subscribe Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPS_CancelReceiveMessage(void* terminal);

/***************************************************************************//**
 * Publishes a message on a Producer Terminal.
 *
 * Sends the given data to all remote Consumer Terminals with the same
 * identifier.
 *
 * @param[in] terminal   Handle of the Producer Terminal
 * @param[in] buffer     Pointer to the beginning of the data to send
 * @param[in] bufferSize Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_PC_Publish(void* terminal, const void* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Asynchronously receives a message published by a remote Producer Terminal.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * a message published by a remote Terminal is received. Messages that are
 * received without an asynchronous receive operation being active are dropped.
 * In order to avoid missing messages, this function should be called again
 * within \p handlerFn.
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, \p handlerFn will be
 * called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p buffer
 * will still be filled with the first \p bufferSize received payload bytes and
 * the size of the received payload parameter will be set to the number of
 * payload bytes in the received message.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  terminal   Handle of the Consumer Terminal
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of \p buffer in bytes
 * @param[in]  handlerFn  Function to call when the message has been received
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_PC_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_PC_AsyncReceiveMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Consumer Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_PC_CancelReceiveMessage(void* terminal);

/***************************************************************************//**
 * Publishes a message on a Cached Producer-Consumer Terminal.
 *
 * Sends the given data to all remote Cached Consumer Terminals.
 *
 * @param[in] terminal   Handle of the Cached Producer Terminal
 * @param[in] buffer     Pointer to the beginning of the data to send
 * @param[in] bufferSize Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPC_Publish(void* terminal, const void* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Retrieves the last message published by a remote Cached Producer Terminal.
 *
 * @param[in]  terminal     Handle of the Cached Producer Terminal
 * @param[out] buffer       Buffer to write the payload to
 * @param[in]  bufferSize   Size of \p buffer in bytes
 * @param[out] bytesWritten Number of bytes written to \p buffer
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPC_GetCachedMessage(void* terminal, void* buffer,
    unsigned bufferSize, unsigned* bytesWritten);

/***************************************************************************//**
 * Asynchronously receives a message published by a remote Cached Produer
 * Terminal.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * a message published by a remote Terminal is received. Messages that are
 * received without an asynchronous receive operation being active are dropped.
 * In order to avoid missing messages, this function should be called again
 * within \p handlerFn.
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, \p handlerFn will be
 * called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p buffer
 * will still be filled with the first \p bufferSize received payload bytes and
 * the size of the received payload parameter will be set to the number of
 * payload bytes in the received message.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# 1 if the received message is a cached one; 0 otherwise
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  terminal   Handle of the Cached Consumer Terminal
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of \p buffer in bytes
 * @param[in]  handlerFn  Function to call when the message has been received
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPC_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, int, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_CPC_AsyncReceiveMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Cached Consumer Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CPC_CancelReceiveMessage(void* terminal);

/***************************************************************************//**
 * Publishes a message on a Master or Slave Terminal.
 *
 * Sends the given data to all remote Slave Terminals if \p terminal is a
 * Master Terminal, or to all remote Master Terminals if \p terminal is a Slave
 * Terminal.
 *
 * @param[in] terminal   Handle of the Master or Slave Terminal
 * @param[in] buffer     Pointer to the beginning of the data to send
 * @param[in] bufferSize Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_MS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Asynchronously receives a message published by a remote Master or Slave
 * Terminal.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * a message published by a remote Terminal is received. Messages that are
 * received without an asynchronous receive operation being active are dropped.
 * In order to avoid missing messages, this function should be called again
 * within \p handlerFn.
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, \p handlerFn will be
 * called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p buffer
 * will still be filled with the first \p bufferSize received payload bytes and
 * the size of the received payload parameter will be set to the number of
 * payload bytes in the received message.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  terminal   Handle of the Master or Slave Terminal
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of \p buffer in bytes
 * @param[in]  handlerFn  Function to call when the message has been received
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_MS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_MS_AsyncReceiveMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Master or Slave Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_MS_CancelReceiveMessage(void* terminal);

/***************************************************************************//**
 * Publishes a message on a Cached Master or Cached Slave Terminal.
 *
 * Sends the given data to all remote Cached Slave Terminals if \p terminal is a
 * Cached Master Terminal, or to all remote Cached Master Terminals if
 * \p terminal is a Cached Slave Terminal.
 *
 * @param[in] terminal   Handle of the Cached Master or Cached Slave Terminal
 * @param[in] buffer     Pointer to the beginning of the data to send
 * @param[in] bufferSize Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CMS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize);

/***************************************************************************//**
 * Retrieves the last message published by a remote Cached Master or Cached
 * Slave Terminal.
 *
 * @param[in]  terminal     Handle of the Cached Master or Cached Slave Terminal
 * @param[out] buffer       Buffer to write the payload to
 * @param[in]  bufferSize   Size of \p buffer in bytes
 * @param[out] bytesWritten Number of bytes written to \p buffer
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CMS_GetCachedMessage(void* terminal, void* buffer,
    unsigned bufferSize, unsigned* bytesWritten);

/***************************************************************************//**
 * Asynchronously receives a message published by a remote Cached Master or
 * Cached Slave Terminal.
 *
 * This function causes the completion handler \p handlerFn to get invoked when
 * a message published by a remote Terminal is received. Messages that are
 * received without an asynchronous receive operation being active are dropped.
 * In order to avoid missing messages, this function should be called again
 * within \p handlerFn.
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, \p handlerFn will be
 * called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p buffer
 * will still be filled with the first \p bufferSize received payload bytes and
 * the size of the received payload parameter will be set to the number of
 * payload bytes in the received message.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# 1 if the received message is a cached one; 0 otherwise
 *  -# Value of the user-defined parameter \p userArg
 *
 * @param[in]  terminal   Handle of the Master or Slave Terminal
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of \p buffer in bytes
 * @param[in]  handlerFn  Function to call when the message has been received
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CMS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, int, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_CMS_AsyncReceiveMessage().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Cached Master or Cached Slave Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_CMS_CancelReceiveMessage(void* terminal);

/***************************************************************************//**
 * Sends a request to the remote Service Terminals and asynchronously waits for
 * them to respond.
 *
 * For each response, the completion handler \p handlerFn will be invoked with
 * the data sent back from the corresponding remote Service Terminal.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Operation ID returned by YOGI_SC_AsyncRequest()
 *  -# Flags (see \ref SCATGATHFLAGS)
 *  -# Size of the received payload (can be greater than \p gathSize)
 *  -# User-defined parameter \p userArg
 *
 * On reception of the data from the very last remote Terminal, the
 * #YOGI_SG_FINISHED flag will be set.
 *
 * If the completion handler \p handlerFn returns #YOGI_DO_STOP, the operation
 * will be aborted and \p handlerFn will not be called any more, even if the
 * #YOGI_SG_FINISHED flag has not been set. In order to continue to receive
 * data, return #YOGI_DO_CONTINUE.
 *
 * If the operation gets canceled via YOGI_SC_CancelRequest(), the \p handlerFn
 * callback function will be called with an error code of #YOGI_ERR_CANCELED
 * and the #YOGI_SG_FINISHED flag set.
 *
 * The target memory location and maximum size for the received message's
 * payload is specified by the parameters \p reqBuf and \p reqSize. If the
 * payload of the received message is larger than \p respSize, the handler
 * function will be called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL.
 * However, \p respBuf will still be filled with the first \p respSize received
 * payload bytes and the size of the received payload parameter will be set to
 * the number of payload bytes in the received message.
 *
 * @param[in]  terminal   Handle of the Client Terminal
 * @param[in]  reqBuf     Pointer to the beginning of the data to send
 * @param[in]  reqSize    Number of bytes to send
 * @param[out] respBuf    Buffer to write gathered data to
 * @param[in]  respSize   Size of \p respBuf in bytes
 * @param[in]  handlerFn  Completion handler
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [>0] ID of the operation if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SC_AsyncRequest(void* terminal, const void* reqBuf,
    unsigned reqSize, void* respBuf, unsigned respSize,
    int (*handlerFn)(int, int, int, unsigned, void*), void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous request operation started via
 * YOGI_SC_AsyncRequest().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal    Handle of the Client Terminal
 * @param[in] operationId ID of the request operation
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SC_CancelRequest(void* terminal, int operationId);

/***************************************************************************//**
 * Asynchronously waits for an incoming requests.
 *
 * The completion handler \p handlerFn gets called once the request message
 * is received. The user code is responsible for reporting back by calling
 * either YOGI_SC_RespondToRequest() or YOGI_SC_IgnoreRequest(). If neither of
 * those functions is called, the request operation on the requesting Terminal
 * will never finish (unless canceled).
 *
 * If no asynchronous receive operation is running while a request is received,
 * the Leaf will automatically report back to the requesting Terminal with the
 * #YOGI_SG_DEAF flag set.
 *
 * The parameters of the completion handler \p handlerFn are:
 *  -# Error code (see \ref ERRORCODES)
 *  -# Operation ID
 *  -# Size of the received payload (can be greater than \p bufferSize)
 *  -# User-defined parameter \p userArg
 *
 * The target memory location and maximum size for the message's payload is
 * specified by the parameters \p buffer and \p bufferSize. If the payload of
 * the received message is larger than \p bufferSize, the handler function will
 * be called with an error code of #YOGI_ERR_BUFFER_TOO_SMALL. However, \p
 * buffer will still be filled with the first \p bufferSize received payload
 * bytes and the size of the received payload parameter will be set to the
 * number of payload bytes in the received message.
 *
 * @param[in]  terminal   Handle of the Service Terminal
 * @param[out] buffer     Buffer to write the payload to
 * @param[in]  bufferSize Size of the buffer in bytes
 * @param[in]  handlerFn  Completion handler
 * @param[in]  userArg    User-defined parameter passed to \p handlerFn
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SC_AsyncReceiveRequest(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, int, unsigned, void*),
    void* userArg);

/***************************************************************************//**
 * Cancels an active asynchronous receive operation started via
 * YOGI_SC_AsyncReceiveRequest().
 *
 * This causes the corresponding completion handler to get called with an error
 * code of #YOGI_ERR_CANCELED.
 *
 * @param[in] terminal Handle of the Service Terminal
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SC_CancelReceiveRequest(void* terminal);

/***************************************************************************//**
 * Responds to a received request by sending back data.
 *
 * @param[in] terminal    Handle of the Service Terminal
 * @param[in] operationId ID of the request operation
 * @param[in] buffer      Pointer to the beginning of the data to send
 * @param[in] bufferSize  Number of bytes to send
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SC_RespondToRequest(void* terminal, int operationId,
    const void* buffer, unsigned bufferSize);

/***************************************************************************//**
 * Ignores a received request.
 *
 * This reports back with the #YOGI_SG_IGNORED flag set.
 *
 * @param[in] terminal    Handle of the Service Terminal
 * @param[in] operationId ID of the request operation
 *
 * @returns [=0] #YOGI_OK if successful
 * @returns [<0] An error code (see \ref ERRORCODES) in case of a failure
 ******************************************************************************/
YOGI_API int YOGI_SC_IgnoreRequest(void* terminal, int operationId);

//! @}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // YOGI_CORE_H
