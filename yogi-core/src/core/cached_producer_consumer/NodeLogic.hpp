#ifndef YOGI_CORE_CACHED_PRODUCER_CONSUMER_NODELOGIC_HPP
#define YOGI_CORE_CACHED_PRODUCER_CONSUMER_NODELOGIC_HPP

#include "../../config.h"
#include "../cached_publish_subscribe/NodeLogic.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace cached_producer_consumer {

/***************************************************************************//**
 * Implements the logic for cached producer-consumer terminals on leafs
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

} // namespace cached_producer_consumer
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_CACHED_PRODUCER_CONSUMER_NODELOGIC_HPP
