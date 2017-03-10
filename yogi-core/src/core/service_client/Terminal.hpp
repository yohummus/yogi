#ifndef CHIRP_CORE_SERVICE_CLIENT_TERMINAL_HPP
#define CHIRP_CORE_SERVICE_CLIENT_TERMINAL_HPP

#include "../../config.h"
#include "logic_types.hpp"
#include "../scatter_gather/Terminal.hpp"


namespace chirp {
namespace core {

class Leaf;

namespace service_client {

/***************************************************************************//**
 * Implements a master-slave terminal
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public scatter_gather::Terminal<TTypes>
{
    typedef scatter_gather::Terminal<TTypes> super;

public:
    Terminal(Leaf& leaf, base::Identifier identifier)
        : super(leaf, identifier)
    {
    }
};

} // namespace service_client
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_SERVICE_CLIENT_TERMINAL_HPP
