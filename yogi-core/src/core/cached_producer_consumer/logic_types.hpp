#ifndef CHIRP_CORE_CACHED_PRODUCER_CONSUMER_LOGIC_TYPES_HPP
#define CHIRP_CORE_CACHED_PRODUCER_CONSUMER_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../cached_publish_subscribe/logic_types.hpp"
#include "../../messaging/messages/CachedProducerConsumer.hpp"


namespace chirp {
namespace core {
namespace cached_producer_consumer {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages=messaging::messages::CachedProducerConsumer>
struct logic_types
    : public cached_publish_subscribe::logic_types<TMessages>
{
    typedef LeafLogic<logic_types> leaf_logic_type;
    typedef NodeLogic<logic_types> node_logic_type;
    typedef Terminal<logic_types> terminal_type;
};

} // namespace cached_producer_consumer
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_CACHED_PRODUCER_CONSUMER_LOGIC_TYPES_HPP
