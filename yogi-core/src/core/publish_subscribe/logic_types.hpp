#ifndef YOGI_CORE_PUBLISH_SUBSCRIBE_LOGIC_TYPES_HPP
#define YOGI_CORE_PUBLISH_SUBSCRIBE_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../common/subscribable_logic_types.hpp"
#include "../../messaging/messages/PublishSubscribe.hpp"


namespace yogi {
namespace core {
namespace publish_subscribe {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages=messaging::messages::PublishSubscribe>
struct logic_types
	: public common::subscribable_logic_types
	, public TMessages
{
	typedef LeafLogic<logic_types> leaf_logic_type;
	typedef NodeLogic<logic_types> node_logic_type;
    typedef Terminal<logic_types> terminal_type;
};

} // namespace publish_subscribe
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_PUBLISH_SUBSCRIBE_LOGIC_TYPES_HPP
