#ifndef YOGI_INTERFACES_ICOMMUNICATOR_HPP
#define YOGI_INTERFACES_ICOMMUNICATOR_HPP

#include "../config.h"
#include "../interfaces/IScheduler.hpp"
#include "IPublicObject.hpp"
#include "IConnection.hpp"


namespace yogi {
namespace interfaces {

/***************************************************************************//**
 * Interface for objects that send and receive messages
 *
 * This interface is used by connections in order to deliver messages to the
 * receiving objects, i.e. nodes and leafs.
 ******************************************************************************/
struct ICommunicator : virtual public IPublicObject
{
    virtual interfaces::IScheduler& scheduler() =0;
    virtual void on_new_connection(IConnection& connection) =0;
    virtual void on_connection_started(IConnection& connection) =0;
    virtual void on_connection_destroyed(IConnection& connection) =0;
    virtual void on_message_received(IMessage&& msg, IConnection& origin) =0;
};

typedef std::shared_ptr<ICommunicator> communicator_ptr;

} // namespace interfaces
} // namespace yogi

#endif // YOGI_INTERFACES_ICOMMUNICATOR_HPP
