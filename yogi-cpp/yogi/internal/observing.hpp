#ifndef YOGI_INTERNAL_OBSERVING_HPP
#define YOGI_INTERNAL_OBSERVING_HPP

#include "../result.hpp"

#include <vector>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <algorithm>

#include <cassert>


namespace yogi {
namespace internal {

template <typename Observer, typename State, typename BadCallbackIdException>
class StateObserver : public Observer
{
public:
    class CallbackId
    {
        friend StateObserver;

    private:
        const StateObserver* m_observer;
        int                  m_id;

    private:
        CallbackId(const StateObserver* observer, int id)
        : m_observer(observer)
        , m_id(id)
        {
        }

    public:
        bool operator== (const CallbackId& other) const
        {
            return m_id == other.m_id;
        }

        bool operator!= (const CallbackId& other) const
        {
            return (*this == other);
        }
    };

private:
    struct Entry {
        CallbackId                  id;
        std::function<void (State)> fn;
    };

private:
    std::recursive_mutex m_mutex;
    std::vector<Entry>   m_callbacks;
    int                  m_idCounter;
    bool                 m_terminate;

private:
    void _on_state_received(const Result& res, State state)
    {
        if (!res) {
            return;
        }

        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        if (m_terminate) {
            return;
        }

        _async_await_state_change([&](auto& res, auto state) {
            this->_on_state_received(res, state);
        });

        for (auto& entry : m_callbacks) {
            entry.fn(state);
        }
    }

    typename std::vector<Entry>::iterator _find_callback(CallbackId id)
    {
        return std::find_if(m_callbacks.begin(), m_callbacks.end(), [&](auto& entry) {
            return entry.id == id;
        });
    }

protected:
    virtual void _async_get_state(std::function<void (const Result&, State)> completionHandler) =0;
    virtual void _async_await_state_change(std::function<void (const Result&, State)> completionHandler) =0;
    virtual void _cancel_await_state() =0;

    void _destroy()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_terminate = true;
        _cancel_await_state();
    }

public:
    StateObserver()
    : m_idCounter(0)
    , m_terminate(false)
    {
    }

    virtual ~StateObserver()
    {
    }

    virtual void start() override
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        _async_get_state([&](auto& res, auto state) {
            this->_on_state_received(res, state);
        });
    }

    virtual void stop() override
    {
        _cancel_await_state();
    }

    CallbackId add(std::function<void (State)> callback)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        CallbackId id(this, m_idCounter++);
        while (_find_callback(id) != m_callbacks.end()) {
            id = CallbackId(this, m_idCounter++);
        }

        m_callbacks.push_back({id, callback});

        return id;
    }

    void remove(CallbackId id)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto it = _find_callback(id);

        if (it == m_callbacks.end()) {
            throw BadCallbackIdException();
        }

        m_callbacks.erase(it);
    }
};


template <typename Observer, typename Terminal, typename PayloadType, typename BadCallbackIdException>
class PublishMessageObserverBase : public Observer
{
public:
    class CallbackId
    {
        friend PublishMessageObserverBase;

    private:
        const PublishMessageObserverBase* m_observer;
        int                               m_id;

    private:
        CallbackId(const PublishMessageObserverBase* observer, int id)
        : m_observer(observer)
        , m_id(id)
        {
        }

    public:
        bool operator== (const CallbackId& other) const
        {
            return m_id == other.m_id;
        }

        bool operator!= (const CallbackId& other) const
        {
            return (*this == other);
        }
    };

private:
    struct EntryWithoutCache {
        CallbackId                               id;
        std::function<void (const PayloadType&)> fn;
    };

    struct EntryWithCache {
        CallbackId                                            id;
        std::function<void (const PayloadType&, cached_flag)> fn;
    };

private:
    Terminal&                      m_terminal;
    std::mutex                     m_mutex;
    std::vector<EntryWithoutCache> m_callbacksWithoutCache;
    std::vector<EntryWithCache>    m_callbacksWithCache;
    int                            m_idCounter;
    bool                           m_terminate;

private:
    CallbackId _make_id()
    {
        CallbackId id(this, m_idCounter++);
        while (std::find_if(m_callbacksWithoutCache.begin(), m_callbacksWithoutCache.end(), [&](auto& entry) { return entry.id == id; }) != m_callbacksWithoutCache.end()
            && std::find_if(m_callbacksWithCache.begin(), m_callbacksWithCache.end(), [&](auto& entry) { return entry.id == id; }) != m_callbacksWithCache.end()) {
                id = CallbackId(this, m_idCounter++);
        }

        return id;
    }

    template <typename Entry>
    bool _try_remove(std::vector<Entry>* callbacks, const CallbackId& id)
    {
        auto it = std::find_if(callbacks->begin(), callbacks->end(), [&](auto& entry) {
            return entry.id == id;
        });

        if (it != callbacks->end()) {
            callbacks->erase(it);
            return true;
        }

        return false;
    }

    void _on_message_received(PayloadType&& payload, cached_flag cached)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_terminate) {
            return;
        }

        _async_receive_message(&Terminal::async_receive_message);

        for (auto& entry : m_callbacksWithoutCache) {
            entry.fn(payload);
        }
        for (auto& entry : m_callbacksWithCache) {
            entry.fn(payload, cached);
        }
    }

    void _async_receive_message(void (Terminal::*fn)(std::function<void (const Result&, PayloadType&&, cached_flag)>))
    {
        (m_terminal.*fn)([&](auto& res, auto&& payload, auto cached) {
            if (res) {
                this->_on_message_received(std::move(payload), cached);
            }
        });
    }

    void _async_receive_message(void (Terminal::*fn)(std::function<void (const Result&, PayloadType&&)>))
    {
        (m_terminal.*fn)([&](auto& res, auto&& payload) {
            if (res) {
                this->_on_message_received(std::move(payload), false);
            }
        });
    }

public:
    PublishMessageObserverBase(Terminal& terminal)
    : m_terminal(terminal)
    , m_idCounter(0)
    , m_terminate(false)
    {
    }

    virtual ~PublishMessageObserverBase()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_terminate = true;
        m_terminal.cancel_receive_message();
    }

    const Terminal& terminal() const
    {
        return m_terminal;
    }

    Terminal& terminal()
    {
        return m_terminal;
    }

    virtual void start() override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        _async_receive_message(&Terminal::async_receive_message);
    }

    virtual void stop() override
    {
        m_terminal.cancel_receive_message();
    }

    CallbackId add(std::function<void (const PayloadType&)> callback)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto id = _make_id();
        m_callbacksWithoutCache.push_back({id, callback});

        return id;
    }

    CallbackId add(std::function<void (const PayloadType&, cached_flag)> callback)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto id = _make_id();
        m_callbacksWithCache.push_back({id, callback});

        return id;
    }

    void remove(CallbackId id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!_try_remove(&m_callbacksWithoutCache, id) && !_try_remove(&m_callbacksWithCache, id)) {
            throw BadCallbackIdException();
        }
    }
};


template <typename Observer, typename Terminal, typename BadCallbackIdException>
class PublishMessageObserver : public PublishMessageObserverBase<Observer, Terminal, typename Terminal::message_type, BadCallbackIdException>
{
public:
    typedef typename Terminal::message_type message_type;

public:
    using PublishMessageObserverBase<Observer, Terminal, typename Terminal::message_type, BadCallbackIdException>::PublishMessageObserverBase;
};


template <typename Observer, typename Terminal, typename BadCallbackIdException>
class MasterMessageObserver : public PublishMessageObserverBase<Observer, Terminal, typename Terminal::master_message_type, BadCallbackIdException>
{
public:
    typedef typename Terminal::master_message_type message_type;

public:
    using PublishMessageObserverBase<Observer, Terminal, typename Terminal::master_message_type, BadCallbackIdException>::PublishMessageObserverBase;
};


template <typename Observer, typename Terminal, typename BadCallbackIdException>
class SlaveMessageObserver : public PublishMessageObserverBase<Observer, Terminal, typename Terminal::slave_message_type, BadCallbackIdException>
{
public:
    typedef typename Terminal::slave_message_type message_type;

public:
    using PublishMessageObserverBase<Observer, Terminal, typename Terminal::slave_message_type, BadCallbackIdException>::PublishMessageObserverBase;
};


template <typename Observer, typename Terminal, typename BadCallbackIdException>
class RawPublishMessageObserver : public PublishMessageObserverBase<Observer, Terminal, std::vector<char>, BadCallbackIdException>
{
public:
    using PublishMessageObserverBase<Observer, Terminal, std::vector<char>, BadCallbackIdException>::PublishMessageObserverBase;
};


template <typename Observer, typename Terminal>
class ScatterMessageObserver : public Observer
{
public:
    typedef typename Terminal::ScatteredMessage ScatteredMessage;

private:
    Terminal&                                    m_terminal;
    std::mutex                                   m_mutex;
    std::function<void (ScatteredMessage&& msg)> m_callbackFn;
    bool                                         m_terminate;

private:
    void _on_message_received(const Result& res, ScatteredMessage&& msg)
    {
        if (!res) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_terminate) {
            return;
        }

        m_terminal.async_receive_scattered_message([&](auto& res, auto&& msg) {
            this->_on_message_received(res, std::move(msg));
        });

        if (res == YOGI_ERR_BUFFER_TOO_SMALL) {
            YOGI_LOG_WARNING(Logger::yogi_logger(), "ScatterMessageObserver<...> could not receive message: " << res);
            msg.ignore();
        }
        else {
            if (m_callbackFn) {
                m_callbackFn(std::move(msg));
            }
            else {
                msg.ignore();
            }
        }
    }

public:
    ScatterMessageObserver(Terminal& terminal)
    : m_terminal(terminal)
    , m_terminate(false)
    {
    }

    virtual ~ScatterMessageObserver()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_terminate = true;
        stop();
    }

    virtual void start() override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_terminal.async_receive_scattered_message([&](auto& res, auto&& msg) {
            this->_on_message_received(res, std::move(msg));
        });
    }

    virtual void stop() override
    {
        m_terminal.cancel_receive_scattered_message();
    }

    void set(std::function<void (ScatteredMessage&&)> callbackFn)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbackFn = callbackFn;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbackFn = decltype(m_callbackFn)();
    }
};


template <typename Observer, typename Terminal>
class RequestObserver : public Observer
{
public:
    typedef typename Terminal::Request Request;

private:
    Terminal&                           m_terminal;
    std::mutex                          m_mutex;
    std::function<void (Request&& req)> m_callbackFn;
    bool                                m_terminate;

private:
    void _on_request_received(const Result& res, Request&& req)
    {
        if (!res) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_terminate) {
            return;
        }

        m_terminal.async_receive_request([&](auto& res, auto&& req) {
            this->_on_request_received(res, std::move(req));
        });

        if (res == YOGI_ERR_BUFFER_TOO_SMALL) {
            YOGI_LOG_WARNING(Logger::yogi_logger(), "RequestObserver<...> could not receive request: " << res);
            req.ignore();
        }
        else {
            if (m_callbackFn) {
                m_callbackFn(std::move(req));
            }
            else {
                req.ignore();
            }
        }
    }

public:
    RequestObserver(Terminal& terminal)
    : m_terminal(terminal)
    , m_terminate(false)
    {
    }

    virtual ~RequestObserver()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_terminate = true;
        stop();
    }

    virtual void start() override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_terminal.async_receive_request([&](auto& res, auto&& req) {
            this->_on_request_received(res, std::move(req));
        });
    }

    virtual void stop() override
    {
        m_terminal.cancel_receive_request();
    }

    void set(std::function<void (Request&&)> callbackFn)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbackFn = callbackFn;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbackFn = decltype(m_callbackFn)();
    }
};

} // namespace internal
} // namespace yogi

#endif // YOGI_INTERNAL_OBSERVING_HPP
