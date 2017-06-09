#ifndef YOGI_CORE_COMMON_LEAFLOGICBASET_HPP
#define YOGI_CORE_COMMON_LEAFLOGICBASET_HPP

#include "../../config.h"
#include "../../interfaces/IConnection.hpp"
#include "../../interfaces/IBinding.hpp"
#include "../../base/IdentifiedObjectRegister.hpp"
#include "../../messaging/fields/fields.hpp"
#include "../../api/ExceptionT.hpp"
#include "LeafLogicBase.hpp"
#include "MappedObjectFsm.hpp"
#include "logic_types.hpp"

#include <boost/log/trivial.hpp>

#include <mutex>
#include <algorithm>


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Templated base class for leaf logic implementations
 ******************************************************************************/
template <typename TTypes>
class LeafLogicBaseT : public LeafLogicBase
{
    typedef LeafLogicBase super;

protected:
    struct terminal_info : public TTypes::leaf_terminal_info_base_type
    {
        MappedObjectFsm                                      fsm;
        typename TTypes::terminal_type*                      terminal;
        mutable typename TTypes::leaf_terminal_info_ext_type ext;
    };

    typedef std::vector<interfaces::IBinding*> bindings_vector;

    struct binding_info : public TTypes::leaf_binding_info_base_type
    {
        MappedObjectFsm                                     fsm;
        bool                                                established = false;
        bindings_vector                                     bindings;
        mutable typename TTypes::leaf_binding_info_ext_type ext;
    };

    typedef base::IdentifiedObjectRegister<terminal_info> terminal_register;
    typedef base::IdentifiedObjectRegister<binding_info>  binding_register;

private:
    const interfaces::scheduler_ptr m_scheduler;

    msg_handler_lut_type     m_msgHandlers;
    std::recursive_mutex     m_mutex;
    interfaces::IConnection* m_connection;
    terminal_register        m_terminals;
    binding_register         m_bindings;

private:
    void update_binding_state(binding_info& bd, bool established)
    {
        if (bd.established != established) {
            bd.established = established;

            on_binding_group_mapping_changed(established, bd);

            auto newState = established
                ? interfaces::IBinding::STATE_ESTABLISHED
                : interfaces::IBinding::STATE_RELEASED;

            for (auto binding : bd.bindings) {
                binding->publish_state(newState);
            }
        }
    }

    void update_binding_state_after_mapping_changed(binding_info& bd,
        bool isMapped)
    {
        if (!isMapped) {
            update_binding_state(bd, false);
        }
        else if (!m_connection->remote_is_node()) {
            update_binding_state(bd, true);
        }
    }

protected:
    explicit LeafLogicBaseT(interfaces::IScheduler& scheduler)
        : m_scheduler {scheduler.make_ptr<interfaces::IScheduler>()}
        , m_connection{nullptr}
    {
        add_msg_handler<typename TTypes::TerminalDescription>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalMapping>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalNoticed>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalRemoved>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalRemovedAck>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingDescription>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingMapping>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingNoticed>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingRemoved>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingRemovedAck>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingEstablished>(this,
            &LeafLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingReleased>(this,
            &LeafLogicBaseT::on_message_received);
    }

    template <typename TMsg, typename TTgt, typename TO>
    void add_msg_handler(TTgt* tgt, void (TO::*fn)(TMsg&&))
    {
        auto msgTypeIdNum = TMsg{}.type_id().number();
        if (m_msgHandlers.size() <= msgTypeIdNum) {
            m_msgHandlers.resize(msgTypeIdNum + 1);
        }

        YOGI_ASSERT(!m_msgHandlers[msgTypeIdNum]);

        m_msgHandlers[msgTypeIdNum] = [=](interfaces::IMessage& msg) {
            auto lock = make_lock_guard();
            (tgt->*fn)(std::move(static_cast<TMsg&>(msg)));
        };
    }

    std::unique_lock<std::recursive_mutex> make_lock_guard()
    {
        return std::unique_lock<std::recursive_mutex>{m_mutex};
    }

    bool connected() const
    {
        return m_connection != nullptr;
    }

    interfaces::IConnection& connection()
    {
        YOGI_ASSERT(connected());
        return *m_connection;
    }

    const msg_handler_lut_type& message_handlers() const
    {
        return m_msgHandlers;
    }

    const terminal_info& get_terminal_info(base::Id id)
    {
        return m_terminals[id];
    }

    const binding_info& get_binding_info(base::Id id)
    {
        return m_bindings[id];
    }

    void on_connection_started(interfaces::IConnection& connection)
    {
		using namespace messaging;

        auto lock = make_lock_guard();

        // store the new connection
        YOGI_ASSERT(m_connection == nullptr);
        m_connection = &connection;

        // send a description of each endpoint and reset the FSM
        for (auto& tm : m_terminals) {
            YOGI_ASSERT(!tm.hidden());
            tm.fsm.reset(!m_connection->remote_is_node(), [&]{
                typename TTypes::TerminalDescription msg;
                msg[fields::identifier] = tm.identifier();
                msg[fields::id]         = tm.id();
                m_connection->send(msg);
            });
        }

        // send a description of each endpoint and reset the FSM
        for (auto& bd : m_bindings) {
            YOGI_ASSERT(!bd.hidden());
            bd.fsm.reset(false, [&] {
                typename TTypes::BindingDescription msg;
                msg[fields::identifier] = bd.identifier();
                msg[fields::id]         = bd.id();
                m_connection->send(msg);
            });
        }
    }

    void on_connection_destroyed()
    {
        auto lock = make_lock_guard();

        // reset the connection pointer
        YOGI_ASSERT(m_connection != nullptr);
        m_connection = nullptr;

        // erase all hidden entries
        for (auto it = m_terminals.begin(); it != m_terminals.end(); ) {
            it = (it->hidden() ? m_terminals.erase(it) : ++it);
        }

        for (auto it = m_bindings.begin(); it != m_bindings.end(); ) {
            it = (it->hidden() ? m_bindings.erase(it) : ++it);
        }

        // set the state of all bindings to RELEASED
        for (auto& bd : m_bindings) {
            update_binding_state(bd, false);
        }

        // reset the existing terminal FSMs
        for (auto it = m_terminals.begin(); it != m_terminals.end(); ++it) {
            bool wasMapped = it->fsm.is_mapped();
            it->fsm.reset(true, [] {});

            if (wasMapped) {
                on_terminal_mapping_changed(false, *it);
            }
        }
    }

    virtual void on_terminal_mapping_changed(bool isMapped,
        const terminal_info& tm)
    {
    }

    virtual void on_terminal_removed(typename TTypes::terminal_type& terminal,
        const terminal_info& tm)
    {
    }

    virtual void on_binding_group_mapping_changed(bool isMapped,
        const binding_info& bd)
    {
    }

    virtual void on_binding_removed(interfaces::IBinding& binding,
        const binding_info& bd)

    {
    }

public:
    base::Id on_new_terminal(typename TTypes::terminal_type& terminal)
    {
		using namespace messaging;

        auto lock = make_lock_guard();

        // create a terminal info entry
        auto res = m_terminals.insert(terminal.identifier());
        auto& tm = *res.first;
        if (!res.second) {
            throw api::ExceptionT<YOGI_ERR_AMBIGUOUS_IDENTIFIER>{};
        }

        tm.terminal = &terminal;

        // send a description of the endpoint and reset the FSM
        if (m_connection) {
            tm.fsm.reset(false, [&] {
                typename TTypes::TerminalDescription msg;
                msg[fields::identifier] = tm.identifier();
                msg[fields::id]         = tm.id();

                m_connection->send(msg);
            });
        }

        return tm.id();
    }

    void on_terminal_destroyed(typename TTypes::terminal_type& terminal)
    {
		using namespace messaging;

        auto lock = make_lock_guard();

        // find the associated terminal info entry
        auto tm = m_terminals.find(terminal.id());
        YOGI_ASSERT(tm != m_terminals.end());

        // reset the terminal pointer
        tm->terminal = nullptr;

        // tell inheriting classes
        on_terminal_removed(terminal, *tm);

        // if we are connected, we evaluate the FSM
        bool eraseEntry = true;
        if (m_connection) {
            eraseEntry = !tm->fsm.process_object_destroyed([&](base::Id mId) {
                typename TTypes::TerminalRemoved msg;
                msg[fields::mappedId] = mId;

                m_connection->send(msg);
            });
        }

        // hide/erase the info
        if (eraseEntry) {
            m_terminals.erase(tm);
        }
        else {
            m_terminals.hide(tm);
        }
    }

    base::Id on_new_binding(interfaces::IBinding& binding)
    {
		using namespace messaging;

        auto lock = make_lock_guard();

        // create a new bindings group if there is no existing one with the same
        // identifier
        auto res = m_bindings.insert(binding.identifier());
        auto& bd = res.first;

        // add the binding to the group
        try {
            bd->bindings.push_back(&binding);
        }
        catch (...) {
            if (bd->bindings.empty()) {
                m_bindings.erase(bd);
            }
            throw;
        }

        // send a description of the binding group and reset the FSM if we
        // created a new binding group
        if (m_connection && bd->bindings.size() == 1) {
            bd->fsm.reset(false, [&] {
                typename TTypes::BindingDescription msg;
                msg[fields::identifier] = bd->identifier();
                msg[fields::id]         = bd->id();

                m_connection->send(msg);
            });
        }
        // else if the bindings in the group are already established, we change
        // the state of the new binding
        else if (bd->established) {
            binding.publish_state(interfaces::IBinding::STATE_ESTABLISHED);
        }

        return bd->id();
    }

    void on_binding_destroyed(interfaces::IBinding& binding)
    {
		using namespace messaging;

        auto lock = make_lock_guard();

        // find the associated binding group entry
        auto bd = m_bindings.find(binding.group_id());
        YOGI_ASSERT(bd != m_bindings.end());

        // erase the binding from the group
        bd->bindings.erase(std::find(bd->bindings.begin(), bd->bindings.end(),
            &binding));

        // tell inheriting classes
        on_binding_removed(binding, *bd);

        // only go on if the binding group is empty now
        if (!bd->bindings.empty())
            return;

        // if we are connected, we evaluate the FSM
        bool eraseEntry = true;
        if (m_connection) {
            eraseEntry = !bd->fsm.process_object_destroyed([&](base::Id mId) {
                typename TTypes::BindingRemoved msg;
                msg[fields::mappedId] = mId;

                m_connection->send(msg);
            });
        }

        // hide/erase the info
        if (eraseEntry) {
            m_bindings.erase(bd);
        }
        else {
            m_bindings.hide(bd);
        }
    }

    void on_message_received(typename TTypes::TerminalDescription&& msg)
    {
		using namespace messaging;

        // search for bindings whose target and signature match the terminal
        auto bd = m_bindings.find(msg[fields::identifier]);

        // no matching local binding => send notification message
        if (bd == m_bindings.end()) {
            typename TTypes::TerminalNoticed reply;
            reply[fields::terminalId] = msg[fields::id];

            m_connection->send(reply);
        }
        // else we evaluate the FSM
        else {
            bd->fsm.process_rcvd_description(msg[fields::id],
                [&] {
                    typename TTypes::TerminalMapping reply;
                    reply[fields::terminalId] = msg[fields::id];
                    reply[fields::mappedId]   = bd->id();

                    m_connection->send(reply);
                },
                [&](base::Id mId) {
                    typename TTypes::TerminalRemovedAck reply;
                    reply[fields::terminalId] = mId;

                    m_connection->send(reply);
                },
                [&](bool isMapped) {
                    update_binding_state_after_mapping_changed(*bd, isMapped);
                }
            );
        }
    }

    void on_message_received(typename TTypes::TerminalMapping&& msg)
    {
		using namespace messaging;

        // find the terminal info entry associated with the terminal ID
        auto& tm = m_terminals[msg[fields::terminalId]];

        // evalutate the state machine
        tm.fsm.process_rcvd_mapping(msg[fields::mappedId],
            [&] {
                typename TTypes::TerminalRemoved reply;
                reply[fields::mappedId] = msg[fields::mappedId];

                m_connection->send(reply);
            },
            [&](base::Id mId) {
                typename TTypes::BindingRemovedAck reply;
                reply[fields::bindingId] = mId;

                m_connection->send(reply);
            },
            [&](bool isMapped) {
                on_terminal_mapping_changed(isMapped, tm);
            }
        );
    }

    void on_message_received(typename TTypes::TerminalNoticed&& msg)
    {
		using namespace messaging;

        // find the terminal info entry associated with the terminal ID
        auto tm = m_terminals.find(msg[fields::terminalId]);
        YOGI_ASSERT(tm != m_terminals.end());

        // evalute the state machine
        bool fsmAlive = tm->fsm.process_rcvd_noticed(
            [&](base::Id mId) {
                typename TTypes::BindingRemovedAck reply;
                reply[fields::bindingId] = mId;

                m_connection->send(reply);
            },
            [&](bool isMapped) {
                on_terminal_mapping_changed(isMapped, *tm);
            }
        );

        // delete the FSM if necessary
        if (!fsmAlive) {
            m_terminals.erase(tm);
        }
    }

    void on_message_received(typename TTypes::TerminalRemoved&& msg)
    {
		using namespace messaging;

        // find the binding group entry associated with the mapped ID
        auto bd = m_bindings.find(msg[fields::mappedId]);
        YOGI_ASSERT(bd != m_bindings.end());

        // evaluate the FSM
        bool fsmAlive = bd->fsm.process_rcvd_removed(
            [&](base::Id mId) {
                typename TTypes::TerminalRemovedAck reply;
                reply[fields::terminalId] = mId;

                m_connection->send(reply);
            },
            [&](bool isMapped) {
                update_binding_state_after_mapping_changed(*bd, isMapped);
            }
        );

        // delete the FSM if necessary
        if (!fsmAlive) {
            m_bindings.erase(bd);
        }
    }

    void on_message_received(typename TTypes::TerminalRemovedAck&& msg)
    {
		using namespace messaging;

        // find the terminal info entry associated with the terminal ID
        auto tm = m_terminals.find(msg[fields::terminalId]);
        YOGI_ASSERT(tm != m_terminals.end());

        // evaluate the FSM
        bool fsmAlive = tm->fsm.process_rcvd_removed_ack();

        // delete the FSM if necessary
        if (!fsmAlive) {
            m_terminals.erase(tm);
        }
    }

    void on_message_received(typename TTypes::BindingDescription&& msg)
    {
		using namespace messaging;

        // search for terminals whose name and signature match the binding
        auto tm = m_terminals.find(msg[fields::identifier]);

        // no matching local endpoint => send notification message
        if (tm == m_terminals.end()) {
            typename TTypes::BindingNoticed reply;
            reply[fields::bindingId] = msg[fields::id];

            m_connection->send(reply);
        }
        // else we evaluate the FSM
        else {
            tm->fsm.process_rcvd_description(msg[fields::id],
                [&] {
                    typename TTypes::BindingMapping reply;
                    reply[fields::bindingId] = msg[fields::id];
                    reply[fields::mappedId]  = tm->id();

                    m_connection->send(reply);
                },
                [&](base::Id mId) {
                    typename TTypes::BindingRemovedAck reply;
                    reply[fields::bindingId] = mId;

                    m_connection->send(reply);
                },
                [&](bool isMapped) {
                    on_terminal_mapping_changed(isMapped, *tm);
                }
            );
        }
    }

    void on_message_received(typename TTypes::BindingMapping&& msg)
    {
		using namespace messaging;

        // find the binding group entry associated with the binding group ID
        auto& bd = m_bindings[msg[fields::bindingId]];

        // evalutate the state machine
        bd.fsm.process_rcvd_mapping(msg[fields::mappedId],
            [&] {
                typename TTypes::BindingRemoved reply;
                reply[fields::mappedId] = msg[fields::mappedId];

                m_connection->send(reply);
            },
            [&](base::Id mId) {
                typename TTypes::TerminalRemovedAck reply;
                reply[fields::terminalId] = mId;

                m_connection->send(reply);
            },
            [&](bool isMapped) {
                update_binding_state_after_mapping_changed(bd, isMapped);
            }
        );
    }

    void on_message_received(typename TTypes::BindingNoticed&& msg)
    {
		using namespace messaging;

        // find the binding group entry associated with the binding group ID
        auto bd = m_bindings.find(msg[fields::bindingId]);
        YOGI_ASSERT(bd != m_bindings.end());

        // evalute the state machine
        bool fsmAlive = bd->fsm.process_rcvd_noticed(
            [&](base::Id mId) {
                typename TTypes::TerminalRemovedAck reply;
                reply[fields::terminalId] = mId;

                m_connection->send(reply);
            },
            [&](bool isMapped) {
                update_binding_state_after_mapping_changed(*bd, isMapped);
            }
        );

        // delete the FSM if necessary
        if (!fsmAlive) {
            m_bindings.erase(bd);
        }
    }

    void on_message_received(typename TTypes::BindingRemoved&& msg)
    {
		using namespace messaging;

        // find the terminal info entry associated with the mapped ID
        auto tm = m_terminals.find(msg[fields::mappedId]);
        YOGI_ASSERT(tm != m_terminals.end());

        // evaluate the FSM
        bool fsmAlive = tm->fsm.process_rcvd_removed(
            [&](base::Id mId) {
                typename TTypes::BindingRemovedAck reply;
                reply[fields::bindingId] = mId;

                m_connection->send(reply);
            },
            [&](bool isMapped) {
                on_terminal_mapping_changed(isMapped, *tm);
            }
        );

        // delete the FSM if necessary
        if (!fsmAlive) {
            m_terminals.erase(tm);
        }
    }

    void on_message_received(typename TTypes::BindingRemovedAck&& msg)
    {
		using namespace messaging;

        // find the binding group entry associated with the binding group ID
        auto bd = m_bindings.find(msg[fields::bindingId]);
        YOGI_ASSERT(bd != m_bindings.end());

        // evaluate the FSM
        bool fsmAlive = bd->fsm.process_rcvd_removed_ack();

        // delete the FSM if necessary
        if (!fsmAlive) {
            m_bindings.erase(bd);
        }
    }

    void on_message_received(typename TTypes::BindingEstablished&& msg)
    {
		using namespace messaging;

        // find the binding group entry associated with the binding group ID
        auto& bd = m_bindings[msg[fields::bindingId]];

        // set the binding group state to ESTABLISHED
        YOGI_ASSERT(bd.fsm.state() == MappedObjectFsm::STATE_MAPPED_CONFIRMED
            || bd.fsm.state() == MappedObjectFsm::STATE_AWAIT_ACK);

        if (bd.fsm.state() == MappedObjectFsm::STATE_MAPPED_CONFIRMED) {
            update_binding_state(bd, true);
        }
    }

    void on_message_received(typename TTypes::BindingReleased&& msg)
    {
		using namespace messaging;

        // find the binding group entry associated with the binding group ID
        auto& bd = m_bindings[msg[fields::bindingId]];

        // set the binding group state to RELEASED
        YOGI_ASSERT(bd.fsm.state() == MappedObjectFsm::STATE_MAPPED_CONFIRMED
            || bd.fsm.state() == MappedObjectFsm::STATE_AWAIT_ACK);

        if (bd.fsm.state() == MappedObjectFsm::STATE_MAPPED_CONFIRMED) {
            update_binding_state(bd, false);
        }
    }
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_LEAFLOGICBASET_HPP
