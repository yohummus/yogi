#ifndef YOGI_INTERFACES_INODE_HPP
#define YOGI_INTERFACES_INODE_HPP

#include "../config.h"
#include "ICommunicator.hpp"


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for nodes
 *
 * A node interconnects leafs and other nodes. Nodes support an arbitrary number
 * of connections and can be seen as hubs/switches in the virtual network.
 ******************************************************************************/
struct INode : public ICommunicator
{
};

typedef std::shared_ptr<INode> node_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_INODE_HPP
