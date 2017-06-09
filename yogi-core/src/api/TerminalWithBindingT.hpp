#ifndef YOGI_API_TERMINALWITHBINDINGT_HPP
#define YOGI_API_TERMINALWITHBINDINGT_HPP

#include "../core/BindingT.hpp"


namespace yogi {
namespace api {

template <typename TTypes>
class TerminalWithBindingT
    : public TTypes::terminal_type
    , public interfaces::IBinding
{
public:
    typedef typename TTypes::terminal_type terminal_type;
    typedef core::BindingT<typename TTypes::leaf_logic_type,
        std::weak_ptr> binding_type;

private:
    std::unique_ptr<binding_type> m_binding;

public:
    TerminalWithBindingT(core::Leaf& leaf,
        base::Identifier::signature_type signature,
        base::Identifier::name_type name, bool terminalHidden)
        : terminal_type(leaf, base::Identifier(signature, name, terminalHidden))
    {
    }

    void create_binding()
    {
        m_binding = std::make_unique<binding_type>(*this, identifier().name(),
            !identifier().hidden());
    }

    virtual interfaces::ITerminal& terminal() override
    {
        return *this;
    }

    virtual const base::Identifier& identifier() const override
    {
        return terminal_type::identifier();
    }

    virtual base::Id group_id() const override
    {
        return m_binding->group_id();
    }

    virtual interfaces::IBinding::state_t state() const override
    {
        return m_binding->state();
    }

    virtual void async_get_state(interfaces::IBinding::handler_fn handlerFn)
        override
    {
        m_binding->async_get_state(handlerFn);
    }

    virtual void async_await_state_change(
        interfaces::IBinding::handler_fn handlerFn) override
    {
        m_binding->async_await_state_change(handlerFn);
    }

    virtual void cancel_await_state_change() override
    {
        m_binding->cancel_await_state_change();
    }

    virtual void publish_state(interfaces::IBinding::state_t state) override
    {
        m_binding->publish_state(state);
    }
};

} // namespace api
} // namespace yogi

#endif // YOGI_API_TERMINALWITHBINDINGT_HPP
