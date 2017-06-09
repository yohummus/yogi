#ifndef YOGI_CORE_DEAF_MUTE_TERMINAL_HPP
#define YOGI_CORE_DEAF_MUTE_TERMINAL_HPP

#include "../../config.h"
#include "logic_types.hpp"
#include "../common/TerminalBaseT.hpp"


namespace yogi {
namespace core {

class Leaf;

namespace deaf_mute {

/***************************************************************************//**
 * Implements a deaf-mute terminal
 *
 * Deaf-mute terminals can be seen as flags which are set if and only if at
 * least one terminal with the specified name exists in the network.
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public common::TerminalBaseT<>
{
public:
    typedef TTypes logic_types;

private:
    const base::Id m_id;

public:
    Terminal(Leaf& leaf, base::Identifier identifier)
        : common::TerminalBaseT<>(leaf, identifier)
        , m_id{register_me<Leaf, LeafLogic<TTypes>>(*this)}
    {
    }

    virtual ~Terminal() override
    {
        deregister_me<Leaf, TTypes>(*this);
    }

    virtual base::Id id() const override
    {
        return m_id;
    }
};

} // namespace deaf_mute
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_DEAF_MUTE_TERMINAL_HPP
