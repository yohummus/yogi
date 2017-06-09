#ifndef YOGI_CORE_SCATTER_GATHER_LOGIC_TYPES_HPP
#define YOGI_CORE_SCATTER_GATHER_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../common/subscribable_logic_types.hpp"
#include "../../messaging/messages/ScatterGather.hpp"

#include <unordered_map>
#include <unordered_set>
#include <algorithm>


namespace yogi {
namespace core {
namespace scatter_gather {

template <typename TTypes>
class LeafLogic;

template <typename TTypes>
class NodeLogic;

template <typename TTypes>
class Terminal;

template <typename TMessages = messaging::messages::ScatterGather>
struct logic_types
	: public common::subscribable_logic_types
	, public TMessages
{
	typedef LeafLogic<logic_types> leaf_logic_type;
	typedef NodeLogic<logic_types> node_logic_type;
	typedef Terminal<logic_types> terminal_type;

    typedef std::unordered_set<base::Id>                operations_set;
    typedef std::unordered_map<base::Id, base::Id>      operations_map;
    typedef std::unordered_multimap<base::Id, base::Id> operations_multimap;

    struct leaf_terminal_info_ext_type
    {
        operations_set scatterOperations; // operation ID
        operations_map gatherOperations;  // operation ID -> binding group ID
    };

    struct leaf_binding_info_ext_type
    {
        operations_multimap gatherOperations; // operation ID -> terminal ID
    };

    struct node_terminal_info_ext_type
    {
        std::unordered_set<base::Id> activeOperations;
    };
};

} // namespace scatter_gather
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_SCATTER_GATHER_LOGIC_TYPES_HPP
