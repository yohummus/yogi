#ifndef CHIRP_CORE_CACHED_PRODUCER_CONSUMER_LEAFLOGIC_HPP
#define CHIRP_CORE_CACHED_PRODUCER_CONSUMER_LEAFLOGIC_HPP

#include "../../config.h"
#include "../cached_publish_subscribe/LeafLogic.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace chirp {
namespace core {
namespace cached_producer_consumer {

/***************************************************************************//**
 * Implements the logic for cached producer-consumer terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public cached_publish_subscribe::LeafLogic<TTypes>
{
public:
    typedef cached_publish_subscribe::LeafLogic<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
    }
};

} // namespace cached_producer_consumer
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_CACHED_PRODUCER_CONSUMER_LEAFLOGIC_HPP
