#ifndef YOGI_CORE_CACHED_MASTER_SLAVE_NODELOGIC_HPP
#define YOGI_CORE_CACHED_MASTER_SLAVE_NODELOGIC_HPP

#include "../../config.h"
#include "../cached_publish_subscribe/NodeLogic.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace cached_master_slave {

/***************************************************************************//**
 * Implements the logic for cached master-slave terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class NodeLogic : public cached_publish_subscribe::NodeLogic<TTypes>
{
    typedef cached_publish_subscribe::NodeLogic<TTypes> super;

protected:
    NodeLogic(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
    }
};

} // namespace cached_master_slave
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_CACHED_MASTER_SLAVE_NODELOGIC_HPP
