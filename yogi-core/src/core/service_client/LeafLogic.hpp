#ifndef YOGI_CORE_SERVICE_CLIENT_LEAFLOGIC_HPP
#define YOGI_CORE_SERVICE_CLIENT_LEAFLOGIC_HPP

#include "../../config.h"
#include "../scatter_gather/LeafLogic.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace service_client {

/***************************************************************************//**
 * Implements the logic for service-client terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public scatter_gather::LeafLogic<TTypes>
{
public:
    typedef scatter_gather::LeafLogic<TTypes> super;

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
    }
};

} // namespace service_client
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_SERVICE_CLIENT_LEAFLOGIC_HPP
