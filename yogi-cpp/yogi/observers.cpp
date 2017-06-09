#include "observers.hpp"
#include "binder.hpp"
#include "subscribable.hpp"
#include "process.hpp"

#include <algorithm>


namespace yogi {

BadCallbackId::BadCallbackId()
: std::runtime_error("No matching callback found")
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
    this->_destroy();
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
    this->_destroy();
}

void OperationalObserver::_async_get_state(std::function<void (const Result&, operational_flag)> completionHandler)
{
    ProcessInterface::_add_operational_observer(this, completionHandler);
}

void OperationalObserver::_async_await_state_change(std::function<void (const Result&, operational_flag)> completionHandler)
{
}

void OperationalObserver::_cancel_await_state()
{
    ProcessInterface::_remove_operational_observer(this);
}

OperationalObserver::OperationalObserver()
{
}

OperationalObserver::~OperationalObserver()
{
    this->_destroy();
}

} // namespace yogi
