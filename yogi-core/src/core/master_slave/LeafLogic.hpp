#ifndef YOGI_CORE_MASTER_SLAVE_LEAFLOGIC_HPP
#define YOGI_CORE_MASTER_SLAVE_LEAFLOGIC_HPP

#include "../../config.h"
#include "../publish_subscribe/LeafLogic.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace master_slave {

/***************************************************************************//**
 * Implements the logic for master-slave terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public publish_subscribe::LeafLogic<TTypes>
{
public:
    typedef publish_subscribe::LeafLogic<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
    }
};

} // namespace master_slave
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_MASTER_SLAVE_LEAFLOGIC_HPP
