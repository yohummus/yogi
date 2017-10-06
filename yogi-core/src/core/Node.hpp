#ifndef YOGI_CORE_NODE_HPP
#define YOGI_CORE_NODE_HPP

#include "../config.h"
#include "../base/AsyncOperation.hpp"
#include "../interfaces/INode.hpp"
#include "deaf_mute/NodeLogic.hpp"
#include "publish_subscribe/NodeLogic.hpp"
#include "scatter_gather/NodeLogic.hpp"
#include "cached_publish_subscribe/NodeLogic.hpp"
#include "producer_consumer/NodeLogic.hpp"
#include "cached_producer_consumer/NodeLogic.hpp"
#include "master_slave/NodeLogic.hpp"
#include "cached_master_slave/NodeLogic.hpp"
#include "service_client/NodeLogic.hpp"

#include <mutex>


namespace yogi {
namespace core {

/***************************************************************************//**
 * Represents a node
 ******************************************************************************/
class Node
    : public interfaces::INode
    , public deaf_mute               ::NodeLogic<>
    , public publish_subscribe       ::NodeLogic<>
    , public scatter_gather          ::NodeLogic<>
    , public cached_publish_subscribe::NodeLogic<>
    , public producer_consumer       ::NodeLogic<>
    , public cached_producer_consumer::NodeLogic<>
    , public master_slave            ::NodeLogic<>
    , public cached_master_slave     ::NodeLogic<>
    , public service_client          ::NodeLogic<>
{
public:
    struct known_terminals_vectors {
        std::vector<base::Identifier> deafMute;
        std::vector<base::Identifier> publishSubscribe;
        std::vector<base::Identifier> scatterGather;
        std::vector<base::Identifier> cachedPublishSubscribe;
        std::vector<base::Identifier> producerConsumer;
        std::vector<base::Identifier> cachedProducerConsumer;
        std::vector<base::Identifier> masterSlave;
        std::vector<base::Identifier> cachedMasterSlave;
        std::vector<base::Identifier> serviceClient;
    };

    struct known_terminal_change_info {
        int              type;
        base::Identifier identifier;
        bool             added;
    };

    typedef std::function<void (const api::Exception&,
        known_terminal_change_info)> await_known_terminals_change_handler_fn;

private:
    const interfaces::scheduler_ptr     m_scheduler;
    NodeLogicBase::msg_handler_lut_type m_msgHandlers;

    std::mutex m_awaitKnownTerminalsChangeOpMutex;
    base::AsyncOperation<await_known_terminals_change_handler_fn>
        m_awaitKnownTerminalsChangeOp;

private:
    void merge_message_handlers(
        const NodeLogicBase::msg_handler_lut_type& others);
    void on_known_terminals_changed(int type, base::Identifier identifier,
        bool added);

public:
    Node(interfaces::IScheduler& scheduler);
    virtual ~Node() override;

    virtual interfaces::IScheduler& scheduler() override;
    virtual void on_new_connection(interfaces::IConnection& connection)
        override;
    virtual void on_connection_started(interfaces::IConnection& connection)
        override;
    virtual void on_connection_destroyed(interfaces::IConnection& connection)
        override;
    virtual void on_message_received(interfaces::IMessage&& msg,
        interfaces::IConnection& origin) override;

    known_terminals_vectors get_known_terminals();
    void async_await_known_terminals_change(
        await_known_terminals_change_handler_fn handlerFn);
    void cancel_await_known_terminals_change();
};

} // namespace core
} // namespace yogi

#endif // YOGI_CORE_NODE_HPP
