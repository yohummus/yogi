#include "subscribable.hpp"
#include "object.hpp"
#include "internal/async.hpp"
#include "internal/utility.hpp"

#include <yogi_core.h>


namespace yogi {

Subscribable::Subscribable(Object* self)
: m_obj(*self)
{
}

subscription_state Subscribable::get_subscription_state() const
{
    int state;
    int res = YOGI_GetSubscriptionState(m_obj.handle(), &state);
    internal::throw_on_failure(res);
    return state == YOGI_SB_SUBSCRIBED ? SUBSCRIBED : UNSUBSCRIBED;
}

void Subscribable::async_get_subscription_state(std::function<void (const Result&, subscription_state)> completionHandler)
{
    internal::async_call<int>([=](const Result& res, int state) {
        completionHandler(res, state == YOGI_SB_SUBSCRIBED ? SUBSCRIBED : UNSUBSCRIBED);
    }, [&](auto fn, void* userArg) {
        return YOGI_AsyncGetSubscriptionState(m_obj.handle(), fn, userArg);
    });
}

void Subscribable::async_await_subscription_state_change(std::function<void (const Result&, subscription_state)> completionHandler)
{
    internal::async_call<int>([=](const Result& res, int state) {
        completionHandler(res, state == YOGI_SB_SUBSCRIBED ? SUBSCRIBED : UNSUBSCRIBED);
    }, [&](auto fn, void* userArg) {
        return YOGI_AsyncAwaitSubscriptionStateChange(m_obj.handle(), fn, userArg);
    });
}

void Subscribable::cancel_await_subscription_state_change()
{
    int res = YOGI_CancelAwaitSubscriptionStateChange(m_obj.handle());
    internal::throw_on_failure(res);
}

} // namespace yogi
