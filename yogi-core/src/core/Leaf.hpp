#ifndef YOGI_CORE_LEAF_HPP
#define YOGI_CORE_LEAF_HPP

#include "../config.h"
#include "../interfaces/ILeaf.hpp"
#include "deaf_mute/LeafLogic.hpp"
#include "publish_subscribe/LeafLogic.hpp"
#include "scatter_gather/LeafLogic.hpp"
#include "cached_publish_subscribe/LeafLogic.hpp"
#include "producer_consumer/LeafLogic.hpp"
#include "cached_producer_consumer/LeafLogic.hpp"
#include "master_slave/LeafLogic.hpp"
#include "cached_master_slave/LeafLogic.hpp"
#include "service_client/LeafLogic.hpp"

#include <mutex>


namespace yogi {
namespace core {

/***************************************************************************//**
 * Represents a leaf
 ******************************************************************************/
class Leaf
    : public interfaces::ILeaf
    , public deaf_mute               ::LeafLogic<>
    , public publish_subscribe       ::LeafLogic<>
    , public scatter_gather          ::LeafLogic<>
    , public cached_publish_subscribe::LeafLogic<>
    , public producer_consumer       ::LeafLogic<>
    , public cached_producer_consumer::LeafLogic<>
    , public master_slave            ::LeafLogic<>
    , public cached_master_slave     ::LeafLogic<>
    , public service_client          ::LeafLogic<>

{
private:
    const interfaces::scheduler_ptr     m_scheduler;
    std::mutex                          m_mutex;
    interfaces::IConnection*            m_connection;
    bool                                m_connectionStarted;
    LeafLogicBase::msg_handler_lut_type m_msgHandlers;

private:
    void merge_message_handlers(
        const LeafLogicBase::msg_handler_lut_type& others);

public:
    Leaf(interfaces::IScheduler& scheduler);

    virtual interfaces::IScheduler& scheduler() override;
    virtual void on_new_connection(interfaces::IConnection& connection)
        override;
    virtual void on_connection_started(interfaces::IConnection& connection)
        override;
    virtual void on_connection_destroyed(interfaces::IConnection& connection)
        override;
    virtual void on_message_received(interfaces::IMessage&& msg,
        interfaces::IConnection& origin) override;
};

} // namespace core
} // namespace yogi

#endif // YOGI_CORE_LEAF_HPP
