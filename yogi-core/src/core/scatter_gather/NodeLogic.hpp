#ifndef YOGI_CORE_SCATTER_GATHER_NODELOGIC_HPP
#define YOGI_CORE_SCATTER_GATHER_NODELOGIC_HPP

#include "../../config.h"
#include "../../base/ObjectRegister.hpp"
#include "../common/SubscribableNodeLogicBaseT.hpp"
#include "logic_types.hpp"


namespace yogi {
namespace core {
namespace scatter_gather {

/***************************************************************************//**
 * Implements the logic for scatter-gather terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class NodeLogic : public common::SubscribableNodeLogicBaseT<TTypes>
{
    typedef common::SubscribableNodeLogicBaseT<TTypes> super;

    struct operation_info_type
    {
        interfaces::IConnection*                     source;
        base::Id                                     sourceOperationId;
        base::Id                                     terminalId;
        std::unordered_set<interfaces::IConnection*> remainingResponses;
    };

private:
    base::ObjectRegister<operation_info_type> m_operations;

protected:
    NodeLogic(interfaces::IScheduler& scheduler,
        typename super::known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
        super::template add_msg_handler<typename TTypes::Scatter>(this,
            &NodeLogic::on_message_received);
        super::template add_msg_handler<typename TTypes::Gather>(this,
            &NodeLogic::on_message_received);
    }

    virtual void on_subscriber_removed(interfaces::IConnection& conn,
        const typename super::terminal_info& tm) override
    {
		using namespace messaging;

		bool connectionIsAlive = super::is_connection_alive(conn);

        auto opIdIt = tm.ext.activeOperations.begin();
        while (opIdIt != tm.ext.activeOperations.end()) {
            auto operationId = *opIdIt;
            auto& op         = m_operations[operationId];

			bool opErased = false;
			if (&conn != op.source) {
				if (!connectionIsAlive || !conn.remote_is_node()) {
					if (op.remainingResponses.erase(&conn)) {
						interfaces::IConnection* opSource = op.source;

                        typename TTypes::Gather msg;
						msg[fields::operationId] = op.sourceOperationId;
						msg[fields::data]        = base::Buffer{};

						if (conn.remote_is_node() || !connectionIsAlive) {
							msg[fields::gatherFlags] = GATHER_CONNECTIONLOST;
						}
						else {
							msg[fields::gatherFlags] = GATHER_BINDINGDESTROYED;
						}

						if (op.remainingResponses.empty()) {
							opIdIt   = tm.ext.activeOperations.erase(opIdIt);
							opErased = true;

							m_operations.erase(operationId);
							msg[fields::gatherFlags] |= GATHER_FINISHED;
						}

						if (opSource) {
							opSource->send(msg);
						}
					}
				}
			}

			if (!opErased) {
				++opIdIt;
			}
        }
    }

    virtual void on_terminal_owner_removed(interfaces::IConnection& conn,
        typename super::const_terminal_iterator tm) override
    {
        super::on_terminal_owner_removed(conn, tm);

        auto operationIt = tm->ext.activeOperations.begin();
        while (operationIt != tm->ext.activeOperations.end()) {
            auto& op = m_operations[*operationIt];
            if (op.source == &conn) {
                op.source = nullptr;
                operationIt = tm->ext.activeOperations.erase(operationIt);
            }
            else {
                ++operationIt;
            }
        }
    }

    void on_message_received(typename TTypes::Scatter&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        auto& tm = super::get_terminal_info(msg[fields::subscriptionId]);

        auto operationId = m_operations.insert();
        auto& op         = m_operations[operationId];

        op.source            = &origin;
        op.sourceOperationId = msg[fields::operationId];
        op.terminalId        = msg[fields::subscriptionId];

        msg[fields::operationId] = operationId;

        for (auto& subscriber : tm.subscribers) {
            auto conn = const_cast<interfaces::IConnection*>(subscriber.first);
            if (conn != &origin) {
                YOGI_ASSERT(tm.usingNodes.count(conn));
                YOGI_ASSERT(tm.usingNodes.find(conn)->second.is_mapped());
                YOGI_ASSERT(!op.remainingResponses.count(conn));

                op.remainingResponses.insert(conn);

                msg[fields::subscriptionId] = subscriber.second;
                conn->send(msg);
            }
        }

        if (tm.binding) {
            auto& bd = super::get_binding_info(tm.binding);

            for (auto& owner : bd.owningLeafs) {
                if (owner.first != &origin) {
                    YOGI_ASSERT(owner.second.is_mapped());
                    YOGI_ASSERT(!op.remainingResponses.count(owner.first));

                    op.remainingResponses.insert(owner.first);

                    msg[fields::subscriptionId] = owner.second.mapped_id();
                    owner.first->send(msg);
                }
            }
        }

		if (op.remainingResponses.empty()) {
			// TODO: check that not sending a Gather message does not cause
			//       a memory leak in the scattering leafs
			m_operations.erase(operationId);
		}
		else {
			YOGI_ASSERT(!tm.ext.activeOperations.count(operationId));
			tm.ext.activeOperations.insert(operationId);
		}
    }

    void on_message_received(typename TTypes::Gather&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        base::Id operationId = msg[fields::operationId];
        auto& op = m_operations[operationId];
        YOGI_ASSERT(op.remainingResponses.count(&origin));

        interfaces::IConnection* opSource = op.source;
        msg[fields::operationId] = op.sourceOperationId;

        if (msg[fields::gatherFlags] & GATHER_FINISHED) {
            op.remainingResponses.erase(&origin);

            if (op.remainingResponses.empty()) {
                if (opSource) {
                    auto& tm = super::get_terminal_info(op.terminalId);
                    YOGI_ASSERT(tm.ext.activeOperations.count(operationId));
                    tm.ext.activeOperations.erase(operationId);
                }

                m_operations.erase(operationId);
            }
            else {
                msg[fields::gatherFlags] &= ~GATHER_FINISHED;
            }
        }

        if (opSource) {
            opSource->send(msg);
        }
    }
};

} // namespace scatter_gather
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_SCATTER_GATHER_NODELOGIC_HPP
