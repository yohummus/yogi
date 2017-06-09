#ifndef YOGI_CORE_COMMON_NODELOGICBASE_HPP
#define YOGI_CORE_COMMON_NODELOGICBASE_HPP

#include "../../config.h"
#include "../../interfaces/IConnection.hpp"

#include <functional>
#include <vector>


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Base class for node logic implementations
 ******************************************************************************/
class NodeLogicBase
{
protected:
    typedef std::function<void (interfaces::IMessage&,
        interfaces::IConnection&)> msg_handler_type;
    typedef std::function<void (base::Identifier, bool)>
        known_terminals_changed_fn;
    typedef std::vector<msg_handler_type> msg_handler_lut_type;

protected:
    virtual ~NodeLogicBase() =default;
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_NODELOGICBASE_HPP
