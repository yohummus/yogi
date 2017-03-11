#ifndef YOGI_CORE_COMMON_SUBSCRIBABLETERMINALBASET_HPP
#define YOGI_CORE_COMMON_SUBSCRIBABLETERMINALBASET_HPP

#include "../../config.h"
#include "../../interfaces/ISubscribableTerminal.hpp"
#include "../../base/AsyncOperation.hpp"
#include "TerminalBaseT.hpp"

#include <boost/asio/strand.hpp>

#include <mutex>
#include <condition_variable>


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Base class for subscribable terminals
 ******************************************************************************/
class SubscribableTerminalBaseT
    : public TerminalBaseT<interfaces::ISubscribableTerminal>
{
private:
    const interfaces::scheduler_ptr m_scheduler;
    const interfaces::leaf_ptr      m_leaf;
    const base::Identifier          m_identifier;
    const base::Id                  m_id;

    mutable std::mutex              m_mutex;
    mutable std::condition_variable m_cv;
    state_t                         m_state;

    bool                             m_getStateActive;
    base::AsyncOperation<handler_fn> m_getStateOp;
    base::AsyncOperation<handler_fn> m_awaitStateChangeOp;

protected:
    SubscribableTerminalBaseT(interfaces::ILeaf& leaf, base::Identifier identifier)
        : TerminalBaseT<interfaces::ISubscribableTerminal>(leaf, identifier)
        , m_scheduler{leaf.scheduler().make_ptr<interfaces::IScheduler>()}
        , m_state{STATE_UNSUBSCRIBED}
        , m_getStateActive{false}
    {
    }

public:
    virtual ~SubscribableTerminalBaseT()
    {
        m_getStateOp        .fire<YOGI_ERR_CANCELED>(state_t{});
        m_awaitStateChangeOp.fire<YOGI_ERR_CANCELED>(state_t{});

        m_getStateOp        .await_idle();
        m_awaitStateChangeOp.await_idle();

        {{
            std::unique_lock<std::mutex> lock{m_mutex};
            m_cv.wait(lock, [&] {
                return !m_getStateActive;
            });
        }}
    }

    virtual state_t subscription_state() const override
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_state;
    }

    virtual void async_get_subscription_state(handler_fn handlerFn) override
    {
        m_getStateOp.arm(handlerFn);

        {{
            std::lock_guard<std::mutex> lock{m_mutex};
            m_getStateActive = true;
        }}

        m_scheduler->post([&] {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_getStateOp.fire<YOGI_OK>(m_state);

            m_getStateActive = false;
            m_cv.notify_all();
        });
    }

    virtual void async_await_subscription_state_change(handler_fn handlerFn) override
    {
        m_awaitStateChangeOp.arm(handlerFn);
    }

    virtual void cancel_await_subscription_state_change() override
    {
        m_awaitStateChangeOp.fire<YOGI_ERR_CANCELED>(state_t{});
    }

    virtual void publish_subscription_state(state_t state) override
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_state = state;

        m_awaitStateChangeOp.fire<YOGI_OK>(state);
    }
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_SUBSCRIBABLETERMINALBASET_HPP
