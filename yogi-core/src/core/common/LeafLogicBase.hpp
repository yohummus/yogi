#ifndef YOGI_CORE_COMMON_LEAFLOGICBASE_HPP
#define YOGI_CORE_COMMON_LEAFLOGICBASE_HPP

#include "../../config.h"
#include "../../interfaces/IMessage.hpp"

#include <functional>
#include <vector>


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Base class for leaf logic implementations
 ******************************************************************************/
class LeafLogicBase
{
protected:
    typedef std::function<void (interfaces::IMessage&)> msg_handler_type;
    typedef std::vector<msg_handler_type>               msg_handler_lut_type;

protected:
    LeafLogicBase() = default;
    virtual ~LeafLogicBase() = default;
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_LEAFLOGICBASE_HPP
