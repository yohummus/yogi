#ifndef YOGI_CORE_MASTER_SLAVE_LOGIC_TYPES_HPP
#define YOGI_CORE_MASTER_SLAVE_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../publish_subscribe/logic_types.hpp"
#include "../../messaging/messages/MasterSlave.hpp"


namespace yogi {
namespace core {
namespace master_slave {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages=messaging::messages::MasterSlave>
struct logic_types
	: public publish_subscribe::logic_types<TMessages>
{
    typedef LeafLogic<logic_types> leaf_logic_type;
    typedef NodeLogic<logic_types> node_logic_type;
    typedef Terminal<logic_types> terminal_type;
};

} // namespace master_slave
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_MASTER_SLAVE_LOGIC_TYPES_HPP
