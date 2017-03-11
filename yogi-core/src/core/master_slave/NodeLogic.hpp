#ifndef YOGI_CORE_MASTER_SLAVE_NODELOGIC_HPP
#define YOGI_CORE_MASTER_SLAVE_NODELOGIC_HPP

#include "../../config.h"
#include "../publish_subscribe/NodeLogic.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace master_slave {

/***************************************************************************//**
 * Implements the logic for master-slave terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class NodeLogic : public publish_subscribe::NodeLogic<TTypes>
{
    typedef publish_subscribe::NodeLogic<TTypes> super;

protected:
    NodeLogic(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
    }
};

} // namespace master_slave
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_MASTER_SLAVE_NODELOGIC_HPP
