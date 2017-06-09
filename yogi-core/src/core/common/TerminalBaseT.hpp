#ifndef YOGI_CORE_COMMON_TERMINALBASET_HPP
#define YOGI_CORE_COMMON_TERMINALBASET_HPP

#include "../../config.h"
#include "../../interfaces/ITerminal.hpp"
#include "../../interfaces/ILeaf.hpp"

#include <functional>


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Base class for terminals
 ******************************************************************************/
template <typename TInterface=interfaces::ITerminal>
class TerminalBaseT : public TInterface
{
private:
    const interfaces::leaf_ptr m_leaf;
    const base::Identifier     m_identifier;
    const base::Id             m_id;

protected:
    TerminalBaseT(interfaces::ILeaf& leaf, base::Identifier identifier)
        : m_leaf      {leaf.make_ptr<interfaces::ILeaf>()}
        , m_identifier{identifier}
    {
    }

    template <typename TLeaf, typename TLeafLogic, typename TTerminal>
    base::Id register_me(TTerminal& me)
    {
        YOGI_ASSERT(dynamic_cast<TLeaf*>(&*m_leaf));
        return static_cast<TLeaf&>(*m_leaf).TLeafLogic::on_new_terminal(me);
    }

    template <typename TLeaf, typename TTypes>
    void deregister_me(typename TTypes::terminal_type& me)
    {
        YOGI_ASSERT(dynamic_cast<TLeaf*>(&*m_leaf));
        static_cast<TLeaf&>(*m_leaf).
            TTypes::leaf_logic_type::on_terminal_destroyed(me);
    }

public:
    virtual interfaces::ILeaf& leaf() override
    {
        return *m_leaf;
    }

    virtual const base::Identifier& identifier() const override
    {
        return m_identifier;
    }
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_TERMINALBASET_HPP
