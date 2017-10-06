#ifndef YOGI_BINDER_HPP
#define YOGI_BINDER_HPP

#include "types.hpp"
#include "result.hpp"

#include <functional>


namespace yogi {

class Object;

class Binder
{
private:
    Object& m_obj;

protected:
    Binder(Object* self);

public:
    binding_state get_binding_state() const;
    void async_get_binding_state(std::function<void (const Result&, binding_state)> completionHandler);
    void async_await_binding_state_change(std::function<void (const Result&, binding_state)> completionHandler);
    void cancel_await_binding_state_change();
};

} // namespace yogi

#endif // YOGI_BINDER_HPP
