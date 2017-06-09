#ifndef YOGI_SUBSCRIBABLE_HPP
#define YOGI_SUBSCRIBABLE_HPP

#include "types.hpp"
#include "result.hpp"

#include <functional>


namespace yogi {

class Object;

class Subscribable
{
private:
    Object& m_obj;

protected:
    Subscribable(Object* self);

public:
    subscription_state get_subscription_state() const;
    void async_get_subscription_state(std::function<void (const Result&, subscription_state)> completionHandler);
    void async_await_subscription_state_change(std::function<void (const Result&, subscription_state)> completionHandler);
    void cancel_await_subscription_state_change();
};

} // namespace yogi

#endif // YOGI_SUBSCRIBABLE_HPP
