#ifndef YOGI_INTERFACES_ISUBSCRIBABLETERMINAL_HPP
#define YOGI_INTERFACES_ISUBSCRIBABLETERMINAL_HPP

#include "../config.h"
#include "../api/ExceptionT.hpp"
#include "ITerminal.hpp"

#include <functional>


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for terminals that can be subscribed to
 ******************************************************************************/
struct ISubscribableTerminal : public ITerminal
{
    enum state_t {
        STATE_UNSUBSCRIBED,
        STATE_SUBSCRIBED
    };

    typedef std::function<void (const api::Exception&, state_t)>
        handler_fn;

    virtual state_t subscription_state() const =0;
    virtual void async_get_subscription_state(handler_fn handlerFn) =0;
    virtual void async_await_subscription_state_change(handler_fn handlerFn) =0;
    virtual void cancel_await_subscription_state_change() =0;
    virtual void publish_subscription_state(state_t state) =0;
};

typedef std::shared_ptr<ISubscribableTerminal> subscribable_terminal_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ISUBSCRIBABLETERMINAL_HPP
