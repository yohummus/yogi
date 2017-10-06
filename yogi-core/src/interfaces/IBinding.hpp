#ifndef YOGI_INTERFACES_IBINDING_HPP
#define YOGI_INTERFACES_IBINDING_HPP

#include "../config.h"
#include "../base/Id.hpp"
#include "../base/Identifier.hpp"
#include "../api/ExceptionT.hpp"
#include "IPublicObject.hpp"

#include <functional>


namespace yogi {
namespace interfaces {

struct ITerminal;

/***************************************************************************//**
 * Interface for bindings
 *
 * Bindings represent a virtual connection between terminals.
 ******************************************************************************/
struct IBinding : virtual public IPublicObject
{
    enum state_t {
        STATE_RELEASED,
        STATE_ESTABLISHED
    };

    typedef std::function<void (const api::Exception&, state_t)>
        handler_fn;

    virtual ITerminal& terminal() =0;
    virtual const base::Identifier& identifier() const =0;
    virtual base::Id group_id() const =0;
    virtual state_t state() const =0;
    virtual void async_get_state(handler_fn handlerFn) =0;
    virtual void async_await_state_change(handler_fn handlerFn) =0;
    virtual void cancel_await_state_change() =0;
    virtual void publish_state(state_t state) =0;
};

typedef std::shared_ptr<IBinding> binding_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_IBINDING_HPP
