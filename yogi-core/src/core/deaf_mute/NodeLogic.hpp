#ifndef YOGI_CORE_DEAF_MUTE_NODELOGIC_HPP
#define YOGI_CORE_DEAF_MUTE_NODELOGIC_HPP

#include "../../config.h"
#include "../common/NodeLogicBaseT.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace deaf_mute {

/***************************************************************************//**
 * Implements the logic for deaf-mute terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class NodeLogic : public common::NodeLogicBaseT<TTypes>
{
    typedef common::NodeLogicBaseT<TTypes> super;

protected:
    NodeLogic(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
    }
};

} // namespace deaf_mute
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_DEAF_MUTE_NODELOGIC_HPP
