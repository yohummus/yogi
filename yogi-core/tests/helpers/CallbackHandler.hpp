#ifndef YOGI_TESTS_HELPERS_CALLBACKHANDLER_HPP
#define YOGI_TESTS_HELPERS_CALLBACKHANDLER_HPP

#include "../../src/yogi_core.h"

#include <mutex>
#include <condition_variable>
#include <functional>


namespace helpers {

class CallbackHandler
{
private:
    mutable std::mutex              m_mutex;
    mutable std::condition_variable m_cv;
    int                             m_callsBeforeWait = 0;
    int                             m_calls = 0;

public:
    void wait(int n = 1)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for (int i = 0; i < n; ++i) {
            m_cv.wait(lock, [&] { return !!m_callsBeforeWait; });
            --m_callsBeforeWait;
        }
    }

    void notify()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_callsBeforeWait;
        ++m_calls;
        m_cv.notify_all();
    }

    int calls() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_calls;
    }
};

struct BindingStateCallbackHandler : public CallbackHandler
{
    int  lastErrorCode  = YOGI_OK;
    int  newState       = -1;
    bool wasReleased    = false;
    bool wasEstablished = false;

    static void fn(int errorCode, int newState, void* userArg)
    {
        auto handler = static_cast<BindingStateCallbackHandler*>(userArg);

        switch (newState) {
        case YOGI_BD_RELEASED:    handler->wasReleased    = true; break;
        case YOGI_BD_ESTABLISHED: handler->wasEstablished = true; break;
        }

        handler->newState      = newState;
        handler->lastErrorCode = errorCode;

        handler->notify();
    }
};

struct SubscriptionStateCallbackHandler : public CallbackHandler
{
    int  lastErrorCode   = YOGI_OK;
    int  newState        = -1;
    bool wasUnsubscribed = false;
    bool wasSubscribed   = false;

    static void fn(int errorCode, int newState, void* userArg)
    {
        auto handler = static_cast<SubscriptionStateCallbackHandler*>(userArg);

        switch (newState) {
        case YOGI_SB_UNSUBSCRIBED: handler->wasUnsubscribed = true; break;
        case YOGI_SB_SUBSCRIBED:   handler->wasSubscribed   = true; break;
        }

        handler->newState      = newState;
        handler->lastErrorCode = errorCode;

        handler->notify();
    }
};

struct ReceivePublishedMessageHandler : public CallbackHandler
{
    int lastErrorCode = YOGI_OK;
    unsigned size     = 0;
    bool cached       = false;

    static void fn(int errorCode, unsigned size, void* userArg)
    {
        auto handler = static_cast<ReceivePublishedMessageHandler*>(userArg);

        handler->lastErrorCode = errorCode;
        handler->size          = size;
        handler->cached        = false;

        handler->notify();
    }

    static void fn(int errorCode, unsigned size, int cached, void* userArg)
    {
        auto handler = static_cast<ReceivePublishedMessageHandler*>(userArg);

        handler->lastErrorCode = errorCode;
        handler->size          = size;
        handler->cached        = !!cached;

        handler->notify();
    }
};

struct ReceiveGatheredMessageHandler : public CallbackHandler
{
    int returnValue   = 0;
    int lastErrorCode = YOGI_OK;
    int operationId   = -1;
    int flags         = YOGI_SG_NOFLAGS;
    unsigned size     = 0;

    static int fn(int errorCode, int operationId, int flags, unsigned size,
        void* userArg)
    {
        auto handler = static_cast<ReceiveGatheredMessageHandler*>(userArg);

        handler->lastErrorCode = errorCode;
        handler->operationId   = operationId;
        handler->flags         = flags;
        handler->size          = size;

        handler->notify();

        return handler->returnValue;
    }
};

struct ReceiveScatteredMessageHandler : public CallbackHandler
{
    int lastErrorCode = YOGI_OK;
    int operationId   = -1;
    unsigned size     = 0;

    static void fn(int errorCode, int operationId, unsigned size, void* userArg)
    {
        auto handler = static_cast<ReceiveScatteredMessageHandler*>(userArg);

        handler->lastErrorCode = errorCode;
        handler->operationId   = operationId;
        handler->size          = size;

        handler->notify();
    }
};

struct AwaitDeathHandler : public CallbackHandler
{
	int lastErrorCode = YOGI_OK;

	static void fn(int errorCode, void* userArg)
	{
		auto handler = static_cast<AwaitDeathHandler*>(userArg);

		handler->lastErrorCode = errorCode;

		handler->notify();
	}
};

struct TcpAcceptHandler : public CallbackHandler
{
	typedef std::function<void (void*)> success_fn;

private:
	const success_fn successFn;

public:
	int   lastErrorCode     = YOGI_OK;
	void* lastTcpConnection = nullptr;

	static void fn(int errorCode, void* connection, void* userArg)
	{
		auto handler = static_cast<TcpAcceptHandler*>(userArg);

		handler->lastErrorCode     = errorCode;
		handler->lastTcpConnection = connection;

		if (errorCode == YOGI_OK && handler->successFn) {
			handler->successFn(connection);
		}

		handler->notify();
	}

	TcpAcceptHandler(success_fn successFn_ = success_fn{})
		: successFn{successFn_}
	{
	}
};

struct TcpConnectHandler : public CallbackHandler
{
	typedef std::function<void (void*)> success_fn;

private:
	const success_fn successFn;

public:
	int   lastErrorCode     = YOGI_OK;
	void* lastTcpConnection = nullptr;

	static void fn(int errorCode, void* connection, void* userArg)
	{
		auto handler = static_cast<TcpConnectHandler*>(userArg);

		handler->lastErrorCode     = errorCode;
		handler->lastTcpConnection = connection;

		if (errorCode == YOGI_OK && handler->successFn) {
			handler->successFn(connection);
		}

		handler->notify();
	}

	TcpConnectHandler(success_fn successFn_ = success_fn{})
		: successFn{successFn_}
	{
	}
};

struct AwaitKnownTerminalsChangeHandler : public CallbackHandler
{
    int lastErrorCode = YOGI_OK;

    static void fn(int errorCode, void* userArg)
    {
        auto handler = static_cast<AwaitKnownTerminalsChangeHandler*>(userArg);

        handler->lastErrorCode = errorCode;

        handler->notify();
    }
};

} // namespace helpers

#endif // YOGI_TESTS_HELPERS_CALLBACKHANDLER_HPP
