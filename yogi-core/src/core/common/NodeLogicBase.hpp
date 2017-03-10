#ifndef CHIRP_CORE_COMMON_NODELOGICBASE_HPP
#define CHIRP_CORE_COMMON_NODELOGICBASE_HPP

#include "../../config.h"
#include "../../interfaces/IConnection.hpp"

#include <functional>
#include <vector>


namespace chirp {
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
} // namespace chirp

#endif // CHIRP_CORE_COMMON_NODELOGICBASE_HPP
