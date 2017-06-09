#ifndef YOGI_CORE_PRODUCER_CONSUMER_LEAFLOGIC_HPP
#define YOGI_CORE_PRODUCER_CONSUMER_LEAFLOGIC_HPP

#include "../../config.h"
#include "../publish_subscribe/LeafLogic.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace producer_consumer {

/***************************************************************************//**
 * Implements the logic for producer-consumer terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public publish_subscribe::LeafLogic<TTypes>
{
public:
    typedef publish_subscribe::LeafLogic<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
    }
};

} // namespace producer_consumer
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_PRODUCER_CONSUMER_LEAFLOGIC_HPP
