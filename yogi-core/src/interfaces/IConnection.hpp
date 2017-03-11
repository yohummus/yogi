#ifndef YOGI_INTERFACES_ICONNECTION_HPP
#define YOGI_INTERFACES_ICONNECTION_HPP

#include "../config.h"
#include "IMessage.hpp"
#include "IConnectionLike.hpp"


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for connections
 *
 * Connections provide the means for leafs and nodes to communicate with each
 * other via variable length messages.
 ******************************************************************************/
struct IConnection : public IConnectionLike
{
    virtual void send(const IMessage& msg) =0;
    virtual bool remote_is_node() const =0;
};

typedef std::shared_ptr<IConnection> connection_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ICONNECTION_HPP
