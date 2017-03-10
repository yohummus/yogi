#ifndef CHIRP_CORE_PRODUCER_CONSUMER_LOGIC_TYPES_HPP
#define CHIRP_CORE_PRODUCER_CONSUMER_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../publish_subscribe/logic_types.hpp"
#include "../../messaging/messages/ProducerConsumer.hpp"


namespace chirp {
namespace core {
namespace producer_consumer {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages=messaging::messages::ProducerConsumer>
struct logic_types
	: public publish_subscribe::logic_types<TMessages>
{
    typedef LeafLogic<logic_types> leaf_logic_type;
    typedef NodeLogic<logic_types> node_logic_type;
    typedef Terminal<logic_types> terminal_type;
};

} // namespace producer_consumer
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_PRODUCER_CONSUMER_LOGIC_TYPES_HPP
