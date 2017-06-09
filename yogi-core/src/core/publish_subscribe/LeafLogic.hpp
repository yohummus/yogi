#ifndef YOGI_CORE_PUBLISH_SUBSCRIBE_LEAFLOGIC_HPP
#define YOGI_CORE_PUBLISH_SUBSCRIBE_LEAFLOGIC_HPP

#include "../../config.h"
#include "../common/PublishSubscribeLeafLogicBaseT.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace publish_subscribe {

/***************************************************************************//**
 * Implements the logic for publish-subscribe terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public common::PublishSubscribeLeafLogicBaseT<TTypes>
{
public:
    typedef common::PublishSubscribeLeafLogicBaseT<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
    }
};

} // namespace publish_subscribe
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_PUBLISH_SUBSCRIBE_LEAFLOGIC_HPP
