#include "binder.hpp"
#include "object.hpp"
#include "internal/async.hpp"
#include "internal/utility.hpp"

#include <yogi_core.h>


namespace yogi {

Binder::Binder(Object* self)
: m_obj(*self)
{
}

binding_state Binder::get_binding_state() const
{
    int state;
    int res = YOGI_GetBindingState(m_obj.handle(), &state);
    internal::throw_on_failure(res);
    return state == YOGI_BD_ESTABLISHED ? ESTABLISHED : RELEASED;
}

void Binder::async_get_binding_state(std::function<void (const Result&, binding_state)> completionHandler)
{
    internal::async_call<int>([=](const Result& res, int state) {
        completionHandler(res, state == YOGI_BD_ESTABLISHED ? ESTABLISHED : RELEASED);
    }, [&](auto fn, void* userArg) {
        return YOGI_AsyncGetBindingState(m_obj.handle(), fn, userArg);
    });
}

void Binder::async_await_binding_state_change(std::function<void (const Result&, binding_state)> completionHandler)
{
    internal::async_call<int>([=](const Result& res, int state) {
        completionHandler(res, state == YOGI_BD_ESTABLISHED ? ESTABLISHED : RELEASED);
    }, [&](auto fn, void* userArg) {
        return YOGI_AsyncAwaitBindingStateChange(m_obj.handle(), fn, userArg);
    });
}

void Binder::cancel_await_binding_state_change()
{
    int res = YOGI_CancelAwaitBindingStateChange(m_obj.handle());
    internal::throw_on_failure(res);
}

} // namespace yogi
