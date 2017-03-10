#ifndef CHIRP_CORE_PRODUCER_CONSUMER_NODELOGIC_HPP
#define CHIRP_CORE_PRODUCER_CONSUMER_NODELOGIC_HPP

#include "../../config.h"
#include "../publish_subscribe/NodeLogic.hpp"
#include "logic_types.hpp"


namespace chirp {
namespace core {
namespace producer_consumer {

/***************************************************************************//**
 * Implements the logic for producer-consumer terminals on leafs
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

} // namespace producer_consumer
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_PRODUCER_CONSUMER_NODELOGIC_HPP
