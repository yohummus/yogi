#ifndef YOGI_INTERFACES_ILEAF_HPP
#define YOGI_INTERFACES_ILEAF_HPP

#include "../config.h"
#include "ICommunicator.hpp"
#include "IBinding.hpp"


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for leafs
 *
 *  A leafs groups terminals together and integrates them into the virtual
 * network. Leafs only support a single connection which can either be connected
 * to another leaf or to a node.
 ******************************************************************************/
struct ILeaf : public ICommunicator
{
};

typedef std::shared_ptr<ILeaf> leaf_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ILEAF_HPP
