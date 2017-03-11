#ifndef YOGI_CORE_COMMON_SUBSCRIBABLE_LOGIC_TYPES_HPP
#define YOGI_CORE_COMMON_SUBSCRIBABLE_LOGIC_TYPES_HPP

#include "../../config.h"
#include "../../interfaces/IConnection.hpp"
#include "../../base/Id.hpp"
#include "logic_types.hpp"

#include <unordered_map>


namespace yogi {
namespace core {
namespace common {

struct subscribable_logic_types : public logic_types
{
    struct leaf_terminal_info_base_type
    {
        mutable bool subscribed = false;
    };

    struct node_terminal_info_base_type
    {
        mutable std::unordered_map<interfaces::IConnection*, base::Id>
            subscribers; // nodes
    };
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_SUBSCRIBABLE_LOGIC_TYPES_HPP
