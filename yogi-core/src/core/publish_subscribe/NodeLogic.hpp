#ifndef YOGI_CORE_PUBLISH_SUBSCRIBE_NODELOGIC_HPP
#define YOGI_CORE_PUBLISH_SUBSCRIBE_NODELOGIC_HPP

#include "../../config.h"
#include "../common/PublishSubscribeNodeLogicBaseT.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace publish_subscribe {

/***************************************************************************//**
 * Implements the logic for publish-subscribe terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class NodeLogic : public common::PublishSubscribeNodeLogicBaseT<TTypes>
{
    typedef common::PublishSubscribeNodeLogicBaseT<TTypes> super;

protected:
    NodeLogic(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
    }
};

} // namespace publish_subscribe
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_PUBLISH_SUBSCRIBE_NODELOGIC_HPP
