#ifndef YOGI_CORE_BINDINGT_HPP
#define YOGI_CORE_BINDINGT_HPP

#include "../config.h"
#include "../base/AsyncOperation.hpp"
#include "../interfaces/IBinding.hpp"
#include "../interfaces/ITerminal.hpp"
#include "../interfaces/IScheduler.hpp"
#include "Leaf.hpp"

#include <boost/asio/strand.hpp>

#include <mutex>
#include <condition_variable>


namespace yogi {
namespace core {

/***************************************************************************//**
 * This class represents a Binding
 ******************************************************************************/
template <typename TLeafLogic,
    template <typename> class TTerminalPointerType = std::shared_ptr>
class BindingT : public interfaces::IBinding
{
private:
    const interfaces::scheduler_ptr                   m_scheduler;
    interfaces::ITerminal&                            m_terminal;
    const TTerminalPointerType<interfaces::ITerminal> m_terminalPointerStore;
    const base::Identifier                            m_identifier;

    base::Id m_groupId;

    mutable std::mutex              m_mutex;
    mutable std::condition_variable m_cv;
    state_t                         m_state;

    bool                             m_getStateActive;
    base::AsyncOperation<handler_fn> m_getStateOp;
    base::AsyncOperation<handler_fn> m_awaitStateChangeOp;

private:
    void cancel_and_await_idle();

public:
    BindingT(interfaces::ITerminal& terminal,
        base::Identifier::name_type targets, bool hiddenTargets)
        : m_scheduler{terminal.leaf().scheduler()
            .make_ptr<interfaces::IScheduler>()}
        , m_terminal{terminal}
        , m_terminalPointerStore{terminal.make_ptr<interfaces::ITerminal>()}
        , m_identifier{terminal.identifier().signature(), targets,
            hiddenTargets}
        , m_state{STATE_RELEASED}
        , m_getStateActive{false}
    {
        YOGI_ASSERT(dynamic_cast<Leaf*>(&m_terminal.leaf()));
        m_groupId = static_cast<Leaf&>(m_terminal.leaf())
            .TLeafLogic::on_new_binding(*this);
    }

public:
    virtual ~BindingT() override
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

        static_cast<Leaf&>(m_terminal.leaf())
            .TLeafLogic::on_binding_destroyed(*this);
    }

    virtual base::Id group_id() const override
    {
        return m_groupId;
    }

    virtual interfaces::ITerminal& terminal() override
    {
        return m_terminal;
    }

    virtual const base::Identifier& identifier() const override
    {
        return m_identifier;
    }

    virtual state_t state() const override
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_state;
    }

    virtual void async_get_state(handler_fn handlerFn) override
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

    virtual void async_await_state_change(handler_fn handlerFn) override
    {
        m_awaitStateChangeOp.arm(handlerFn);
    }

    virtual void cancel_await_state_change() override
    {
        m_awaitStateChangeOp.fire<YOGI_ERR_CANCELED>(state_t{});
    }

    virtual void publish_state(state_t state) override
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_state = state;

        m_awaitStateChangeOp.fire<YOGI_OK>(state);
    }
};

} // namespace core
} // namespace yogi

#endif // YOGI_CORE_BINDINGT_HPP
