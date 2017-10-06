#include "Node.hpp"
#include "../api/ExceptionT.hpp"

#include <vector>

using namespace std::placeholders;


namespace yogi {
namespace core {

void Node::merge_message_handlers(
    const NodeLogicBase::msg_handler_lut_type& others)
{
    if (m_msgHandlers.size() < others.size()) {
        m_msgHandlers.resize(others.size());
    }

    for (std::size_t i = 0; i < others.size(); ++i){
        if (others[i]) {
            m_msgHandlers[i] = others[i];
        }
    }
}

void Node::on_known_terminals_changed(int type, base::Identifier identifier,
    bool added)
{
    known_terminal_change_info info;
    info.type       = type;
    info.identifier = identifier;
    info.added      = added;

    std::lock_guard<std::mutex> lock{m_awaitKnownTerminalsChangeOpMutex};

    m_awaitKnownTerminalsChangeOp.fire<YOGI_OK>(info);
}

Node::Node(interfaces::IScheduler& scheduler)
    : deaf_mute::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_DEAFMUTE, _1, _2)}
    , publish_subscribe::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_PUBLISHSUBSCRIBE, _1, _2)}
    , scatter_gather::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_SCATTERGATHER, _1, _2)}
    , cached_publish_subscribe::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_CACHEDPUBLISHSUBSCRIBE, _1,
        _2)}
    , producer_consumer::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_PRODUCER, _1, _2)}
    , cached_producer_consumer::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_CACHEDPRODUCER, _1,
        _2)}
    , master_slave::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_MASTER, _1, _2)}
    , cached_master_slave::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_CACHEDMASTER, _1,
        _2)}
    , service_client::NodeLogic<>{scheduler, std::bind(&Node
        ::on_known_terminals_changed, this, YOGI_TM_SERVICE, _1, _2)}
    , m_scheduler{scheduler.make_ptr<interfaces::IScheduler>()}
{
    merge_message_handlers(deaf_mute
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(publish_subscribe
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(scatter_gather
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(cached_publish_subscribe
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(producer_consumer
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(cached_producer_consumer
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(master_slave
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(cached_master_slave
        ::NodeLogic<>::message_handlers());
    merge_message_handlers(service_client
        ::NodeLogic<>::message_handlers());
}

Node::~Node()
{
    m_awaitKnownTerminalsChangeOp.fire<YOGI_ERR_CANCELED>(
        known_terminal_change_info{});
    m_awaitKnownTerminalsChangeOp.await_idle();
}

interfaces::IScheduler& Node::scheduler()
{
    return *m_scheduler;
}

void Node::on_new_connection(interfaces::IConnection& connection)
{
}

void Node::on_connection_started(interfaces::IConnection& connection)
{
#define YOGI_CONNECTION_STARTED(type_namespace, try_body)                     \
    type_namespace::NodeLogic<>::on_connection_started(connection);            \
    try {                                                                      \
        try_body                                                               \
    }                                                                          \
    catch (...) {                                                              \
        type_namespace::NodeLogic<>::on_connection_destroyed(connection);      \
        throw;                                                                 \
    }

    YOGI_CONNECTION_STARTED(deaf_mute,
        YOGI_CONNECTION_STARTED(publish_subscribe,
        YOGI_CONNECTION_STARTED(scatter_gather,
        YOGI_CONNECTION_STARTED(cached_publish_subscribe,
        YOGI_CONNECTION_STARTED(producer_consumer,
        YOGI_CONNECTION_STARTED(cached_producer_consumer,
        YOGI_CONNECTION_STARTED(master_slave,
        YOGI_CONNECTION_STARTED(cached_master_slave,
        YOGI_CONNECTION_STARTED(service_client,
    )))))))));

#undef YOGI_CONNECTION_STARTED
}

void Node::on_connection_destroyed(interfaces::IConnection& connection)
{
    deaf_mute
        ::NodeLogic<>::on_connection_destroyed(connection);
    publish_subscribe
        ::NodeLogic<>::on_connection_destroyed(connection);
    scatter_gather
        ::NodeLogic<>::on_connection_destroyed(connection);
    cached_publish_subscribe
        ::NodeLogic<>::on_connection_destroyed(connection);
    producer_consumer
        ::NodeLogic<>::on_connection_destroyed(connection);
    cached_producer_consumer
        ::NodeLogic<>::on_connection_destroyed(connection);
    master_slave
        ::NodeLogic<>::on_connection_destroyed(connection);
    cached_master_slave
        ::NodeLogic<>::on_connection_destroyed(connection);
    service_client
        ::NodeLogic<>::on_connection_destroyed(connection);
}

void Node::on_message_received(interfaces::IMessage&& msg,
    interfaces::IConnection& origin)
{
    m_msgHandlers[msg.type_id().number()](msg, origin);
}

Node::known_terminals_vectors Node::get_known_terminals()
{
    known_terminals_vectors v;
    v.deafMute = deaf_mute
        ::NodeLogic<>::get_known_terminals();
    v.publishSubscribe = publish_subscribe
        ::NodeLogic<>::get_known_terminals();
    v.scatterGather = scatter_gather
        ::NodeLogic<>::get_known_terminals();
    v.cachedPublishSubscribe = cached_publish_subscribe
        ::NodeLogic<>::get_known_terminals();
    v.producerConsumer = producer_consumer
        ::NodeLogic<>::get_known_terminals();
    v.cachedProducerConsumer = cached_producer_consumer
        ::NodeLogic<>::get_known_terminals();
    v.masterSlave = master_slave
        ::NodeLogic<>::get_known_terminals();
    v.cachedMasterSlave = cached_master_slave
        ::NodeLogic<>::get_known_terminals();
    v.serviceClient = service_client
        ::NodeLogic<>::get_known_terminals();

    return v;
}

void Node::async_await_known_terminals_change(
    await_known_terminals_change_handler_fn handlerFn)
{
    m_awaitKnownTerminalsChangeOp.arm(handlerFn);
}

void Node::cancel_await_known_terminals_change()
{
    m_awaitKnownTerminalsChangeOp.fire<YOGI_ERR_CANCELED>(
        known_terminal_change_info{});
}

} // namespace core
} // namespace yogi
