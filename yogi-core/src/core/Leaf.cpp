#include "Leaf.hpp"
#include "../api/ExceptionT.hpp"

#include <vector>


namespace yogi {
namespace core {

void Leaf::merge_message_handlers(
    const LeafLogicBase::msg_handler_lut_type& others)
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

Leaf::Leaf(interfaces::IScheduler& scheduler)
    : deaf_mute               ::LeafLogic<>{scheduler}
    , publish_subscribe       ::LeafLogic<>{scheduler}
    , scatter_gather          ::LeafLogic<>{scheduler}
    , cached_publish_subscribe::LeafLogic<>{scheduler}
    , producer_consumer       ::LeafLogic<>{scheduler}
    , cached_producer_consumer::LeafLogic<>{scheduler}
    , master_slave            ::LeafLogic<>{scheduler}
    , cached_master_slave     ::LeafLogic<>{scheduler}
    , service_client          ::LeafLogic<>{scheduler}
    , m_scheduler{scheduler.make_ptr<interfaces::IScheduler>()}
    , m_connection{nullptr}
    , m_connectionStarted{false}
{
    merge_message_handlers(deaf_mute
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(publish_subscribe
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(scatter_gather
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(cached_publish_subscribe
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(producer_consumer
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(cached_producer_consumer
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(master_slave
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(cached_master_slave
        ::LeafLogic<>::message_handlers());
    merge_message_handlers(service_client
        ::LeafLogic<>::message_handlers());
}

interfaces::IScheduler& Leaf::scheduler()
{
    return *m_scheduler;
}

void Leaf::on_new_connection(interfaces::IConnection& connection)
{
    std::lock_guard<std::mutex> lock{m_mutex};
    if (m_connection) {
        throw api::ExceptionT<YOGI_ERR_ALREADY_CONNECTED>{};
    }
    else {
        m_connection = &connection;
    }
}

void Leaf::on_connection_started(interfaces::IConnection& connection)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    YOGI_ASSERT(m_connection == &connection);

#define YOGI_CONNECTION_STARTED(type_namespace, try_body)                     \
    type_namespace::LeafLogic<>::on_connection_started(connection);            \
    try {                                                                      \
        try_body                                                               \
    }                                                                          \
    catch (...) {                                                              \
        type_namespace::LeafLogic<>::on_connection_destroyed();                \
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

    m_connectionStarted = true;
}

void Leaf::on_connection_destroyed(interfaces::IConnection& connection)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    m_connection = nullptr;

    if (m_connectionStarted) {
        deaf_mute
            ::LeafLogic<>::on_connection_destroyed();
        publish_subscribe
            ::LeafLogic<>::on_connection_destroyed();
        scatter_gather
            ::LeafLogic<>::on_connection_destroyed();
        cached_publish_subscribe
            ::LeafLogic<>::on_connection_destroyed();
        producer_consumer
            ::LeafLogic<>::on_connection_destroyed();
        cached_producer_consumer
            ::LeafLogic<>::on_connection_destroyed();
        master_slave
            ::LeafLogic<>::on_connection_destroyed();
        cached_master_slave
            ::LeafLogic<>::on_connection_destroyed();
        service_client
            ::LeafLogic<>::on_connection_destroyed();

        m_connectionStarted = false;
    }
}

void Leaf::on_message_received(interfaces::IMessage&& msg,
    interfaces::IConnection&)
{
    m_msgHandlers[msg.type_id().number()](msg);
}

} // namespace core
} // namespace yogi
