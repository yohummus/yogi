#ifndef YOGI_CORE_SERVICE_CLIENT_LOGIC_TYPES_HPP
#define YOGI_CORE_SERVICE_CLIENT_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../scatter_gather/logic_types.hpp"
#include "../../messaging/messages/ServiceClient.hpp"


namespace yogi {
namespace core {
namespace service_client {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages=messaging::messages::ServiceClient>
struct logic_types
	: public scatter_gather::logic_types<TMessages>
{
    typedef LeafLogic<logic_types> leaf_logic_type;
    typedef NodeLogic<logic_types> node_logic_type;
    typedef Terminal<logic_types> terminal_type;
};

} // namespace service_client
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_SERVICE_CLIENT_LOGIC_TYPES_HPP
