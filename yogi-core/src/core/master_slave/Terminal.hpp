#ifndef YOGI_CORE_MASTER_SLAVE_TERMINAL_HPP
#define YOGI_CORE_MASTER_SLAVE_TERMINAL_HPP

#include "../../config.h"
#include "../publish_subscribe/Terminal.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {

class Leaf;

namespace master_slave {

/***************************************************************************//**
 * Implements a master-slave terminal
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public publish_subscribe::Terminal<TTypes>
{
    typedef publish_subscribe::Terminal<TTypes> super;

public:
    Terminal(Leaf& leaf, base::Identifier identifier)
        : super(leaf, identifier)
    {
    }
};

} // namespace master_slave
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_MASTER_SLAVE_TERMINAL_HPP
