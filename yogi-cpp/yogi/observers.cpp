#include "observers.hpp"
#include "binder.hpp"
#include "subscribable.hpp"
#include "process.hpp"
#include "errors.hpp"

#include <algorithm>


namespace yogi {

const char* CalledFromHandler::what() const noexcept
{
    return "Function cannot be called from within a handler function";
}

const char* BadCallbackId::what() const noexcept
{
    return "No matching callback found";
}

std::unique_lock<std::recursive_mutex> Observer::_make_lock()
{
    return std::unique_lock<std::recursive_mutex>(m_mutex);
}

std::unique_lock<std::recursive_mutex> Observer::_make_lock_outside_of_handler()
{
    auto lock = _make_lock();

    if (m_inCallback) {
        throw CalledFromHandler();
    }

    return std::move(lock);
}

void Observer::start()
{
    auto lock = _make_lock_outside_of_handler();

    if (!m_running) {
        _start_impl();
        m_running = true;
        m_stop = false;
    }
}

void Observer::stop()
{
    {{
    auto lock = _make_lock_outside_of_handler();
    m_stop = true;
    }}

    _stop_impl(); // not holding lock to avoid deadlock

    while (true) {
        auto lock = this->_make_lock();
        if (!m_running) {
            break;
        }
    }
}

Observer::Observer()
: m_running(false)
, m_stop(false)
, m_inCallback(false)
{
}

void BindingObserver::_async_get_state(std::function<void (const Result&, binding_state)> completionHandler)
{
    m_binder.async_get_binding_state(completionHandler);
}

void BindingObserver::_async_await_state_change(std::function<void (const Result&, binding_state)> completionHandler)
{
    m_binder.async_await_binding_state_change(completionHandler);
}

void BindingObserver::_cancel_await_state()
{
    m_binder.cancel_await_binding_state_change();
}

BindingObserver::BindingObserver(Binder& binder)
: m_binder(binder)
{
}

BindingObserver::~BindingObserver()
{
    this->stop();

    // make sure m_mutex exists as long as it is used by _on_message_received()
    auto lock = this->_make_lock();
}

void SubscriptionObserver::_async_get_state(std::function<void (const Result&, subscription_state)> completionHandler)
{
    m_subscribable.async_get_subscription_state(completionHandler);
}

void SubscriptionObserver::_async_await_state_change(std::function<void (const Result&, subscription_state)> completionHandler)
{
    m_subscribable.async_await_subscription_state_change(completionHandler);
}

void SubscriptionObserver::_cancel_await_state()
{
    m_subscribable.cancel_await_subscription_state_change();
}

SubscriptionObserver::SubscriptionObserver(Subscribable& subscribable)
: m_subscribable(subscribable)
{
}

SubscriptionObserver::~SubscriptionObserver()
{
    this->stop();

    // make sure m_mutex exists as long as it is used by _on_message_received()
    auto lock = this->_make_lock();
}

void OperationalObserver::_async_get_state(std::function<void (const Result&, operational_flag)> completionHandler)
{
    ProcessInterface::_add_operational_observer(this, completionHandler);
    m_completionHandler = completionHandler;
}

void OperationalObserver::_async_await_state_change(std::function<void (const Result&, operational_flag)> completionHandler)
{
    m_completionHandler = completionHandler;
}

void OperationalObserver::_cancel_await_state()
{
    ProcessInterface::_remove_operational_observer(this);
    m_completionHandler(errors::Canceled(), {});
}

OperationalObserver::OperationalObserver()
{
}

OperationalObserver::~OperationalObserver()
{
    this->stop();

    // make sure m_mutex exists as long as it is used by _on_message_received()
    auto lock = this->_make_lock();
}

} // namespace yogi
