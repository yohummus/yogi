#ifndef YOGI_CORE_CACHED_PRODUCER_CONSUMER_TERMINAL_HPP
#define YOGI_CORE_CACHED_PRODUCER_CONSUMER_TERMINAL_HPP

#include "../../config.h"
#include "../cached_publish_subscribe/Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {

class Leaf;

namespace cached_producer_consumer {

/***************************************************************************//**
 * Implements a cached producer-consumer terminal
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public cached_publish_subscribe::Terminal<TTypes>
{
    typedef cached_publish_subscribe::Terminal<TTypes> super;

public:
    Terminal(Leaf& leaf, base::Identifier identifier)
        : super(leaf, identifier)
    {
    }
};

} // namespace cached_producer_consumer
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_CACHED_PRODUCER_CONSUMER_TERMINAL_HPP
