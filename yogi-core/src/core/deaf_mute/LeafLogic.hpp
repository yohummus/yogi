#ifndef YOGI_CORE_DEAF_MUTE_LEAFLOGIC_HPP
#define YOGI_CORE_DEAF_MUTE_LEAFLOGIC_HPP

#include "../../config.h"
#include "../common/LeafLogicBaseT.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace deaf_mute {

/***************************************************************************//**
 * Implements the logic for deaf-mute terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public common::LeafLogicBaseT<TTypes>
{
public:
    typedef common::LeafLogicBaseT<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
    }
};

} // namespace deaf_mute
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_DEAF_MUTE_LEAFLOGIC_HPP
