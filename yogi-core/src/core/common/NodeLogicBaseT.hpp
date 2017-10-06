#ifndef YOGI_CORE_COMMON_NODELOGICBASET_HPP
#define YOGI_CORE_COMMON_NODELOGICBASET_HPP

#include "../../config.h"
#include "../../interfaces/IScheduler.hpp"
#include "../../interfaces/IConnection.hpp"
#include "../../base/IdentifiedObjectRegister.hpp"
#include "../../messaging/fields/fields.hpp"
#include "NodeLogicBase.hpp"
#include "MappedObjectFsm.hpp"
#include "logic_types.hpp"

#include <boost/log/trivial.hpp>

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>


namespace yogi {
namespace core {
namespace common {

/***************************************************************************//**
 * Templated base class for node logic implementations
 ******************************************************************************/
template <typename TTypes>
class NodeLogicBaseT : public NodeLogicBase
{
    typedef NodeLogicBase super;

protected:
    typedef std::unordered_map<interfaces::IConnection*, MappedObjectFsm>
        fsm_map;
    typedef std::unordered_set<interfaces::IConnection*> connections_set;

    enum state_t {
        STATE_JUST_CREATED,
        STATE_OWNERS_1_LEAFS_0_NODES,
        STATE_OWNERS_M_LEAFS_0_NODES, // M > 1
        STATE_OWNERS_0_LEAFS_1_NODES,
        STATE_OWNERS_0_LEAFS_M_NODES, // M > 1
        STATE_OWNERS_N_LEAFS_N_NODES, // N > 0
        STATE_AWAIT_ACKS
    };

    struct terminal_info : public TTypes::node_terminal_info_base_type
    {
        state_t  state = STATE_JUST_CREATED;
        fsm_map  owningLeafs; // leafs that have such a terminal
        fsm_map  owningNodes; // nodes that have such a terminal
        fsm_map  usingNodes;  // nodes that use the terminal
        base::Id binding;     // ID of the associated local binding
        mutable typename TTypes::node_terminal_info_ext_type ext;
    };

    struct binding_info : public TTypes::node_binding_info_base_type
    {
        fsm_map  owningLeafs; // leafs that have such a binding
        base::Id terminal;    // ID of the associated local terminal
        mutable typename TTypes::node_binding_info_ext_type ext;
    };

protected:
    typedef base::IdentifiedObjectRegister<terminal_info>    terminal_register;
    typedef typename terminal_register::iterator             terminal_iterator;
    typedef typename terminal_register::const_iterator const_terminal_iterator;

    typedef base::IdentifiedObjectRegister<binding_info>      binding_register;
    typedef typename binding_register::iterator               binding_iterator;
    typedef typename binding_register::const_iterator   const_binding_iterator;

private:
    const interfaces::scheduler_ptr  m_scheduler;
    const known_terminals_changed_fn m_knownTerminalsChangedFn;

    msg_handler_lut_type m_msgHandlers;

    std::mutex        m_mutex;
    connections_set   m_leafConnections;
    connections_set   m_nodeConnections;
    terminal_register m_terminals;
    binding_register  m_bindings;

private:
    binding_iterator associate(terminal_iterator tm)
    {
        if (tm->binding) {
            YOGI_ASSERT(m_bindings.find(tm->binding) != m_bindings.end());
            YOGI_ASSERT(m_bindings.find(tm->binding)->terminal == tm->id());
            return m_bindings.find(tm->binding);
        }

        auto bd = m_bindings.find(tm->identifier());
        if (bd != m_bindings.end()) {
            bd->terminal = tm->id();
            tm->binding  = bd->id();
        }

        return bd;
    }

    terminal_iterator associate(binding_iterator bd)
    {
        if (bd->terminal) {
            YOGI_ASSERT(m_terminals.find(bd->terminal) != m_terminals.end());
            YOGI_ASSERT(m_terminals.find(bd->terminal)->binding == bd->id());
            return m_terminals.find(bd->terminal);
        }

        auto tm = m_terminals.find(bd->identifier());
        if (tm != m_terminals.end()) {
            tm->binding  = bd->id();
            bd->terminal = tm->id();
        }

        return tm;
    }

    void disassociate(terminal_iterator tm)
    {
        if (tm->binding) {
            YOGI_ASSERT(m_bindings.find(tm->binding) != m_bindings.end());
            m_bindings[tm->binding].terminal = base::Id{};
            tm->binding = base::Id{};
        }
    }

    void disassociate(binding_iterator bd)
    {
        if (bd->terminal) {
            m_terminals[bd->terminal].binding = base::Id{};
            bd->terminal = base::Id{};
        }
    }

    fsm_map& ownersMap(terminal_iterator tm,
        const interfaces::IConnection& connection)
    {
        if (connection.remote_is_node()) {
            return tm->owningNodes;
        }
        else {
            return tm->owningLeafs;
        }
    }

    bool update_state(terminal_iterator tm)
    {
        if (tm->owningLeafs.empty() && tm->owningNodes.empty() &&
            tm->usingNodes.empty()) {
                // inform library user that we no longer know the terminal
                if (!tm->identifier().hidden()) {
                    m_knownTerminalsChangedFn(tm->identifier(), false);
                }

                m_terminals.erase(tm);
                return false;
        }

        switch (tm->owningLeafs.size()) {
        case 0:
            switch (tm->owningNodes.size()) {
            case 0:  tm->state = STATE_AWAIT_ACKS;             break;
            case 1:  tm->state = STATE_OWNERS_0_LEAFS_1_NODES; break;
            default: tm->state = STATE_OWNERS_0_LEAFS_M_NODES; break;
            }
            break;
        case 1:
            switch (tm->owningNodes.size()) {
            case 0:  tm->state = STATE_OWNERS_1_LEAFS_0_NODES; break;
            case 1:  tm->state = STATE_OWNERS_N_LEAFS_N_NODES; break;
            default: tm->state = STATE_OWNERS_N_LEAFS_N_NODES; break;
            }
            break;
        default:
            switch (tm->owningNodes.size()) {
            case 0:  tm->state = STATE_OWNERS_M_LEAFS_0_NODES; break;
            case 1:  tm->state = STATE_OWNERS_N_LEAFS_N_NODES; break;
            default: tm->state = STATE_OWNERS_N_LEAFS_N_NODES; break;
            }
            break;
        }

        return true;
    }

    void update_users_for_abandoned_terminal(terminal_iterator tm,
        const interfaces::IConnection* exception = nullptr)
    {
		using namespace messaging;

        auto it = tm->usingNodes.begin();
        while (it != tm->usingNodes.end()) {
            if (it->first == exception || it->second.destroyed()) {
                ++it;
                continue;
            }

            bool alive = it->second.process_object_destroyed(
                [&](base::Id mappedId) {
                    typename TTypes::TerminalRemoved msg;
                    msg[fields::mappedId] = mappedId;

                    it->first->send(msg);
                }
            );

            if (alive) {
                ++it;
            }
            else {
                auto& connection = *it->first;
                it = tm->usingNodes.erase(it);
                on_terminal_user_removed(connection, tm);
            }
        }
    }

    void remove_terminal_owner(terminal_iterator tm,
        interfaces::IConnection& connection, bool brokenConnection)
    {
		using namespace messaging;

        // find the owner entry
        auto& owners = ownersMap(tm, connection);
        auto ownerIt = owners.find(&connection);

        if (brokenConnection && ownerIt == owners.end()) {
            return;
        }

        // update the terminal FSM
        if (!brokenConnection) {
            bool stillOwned = true;
            ownerIt->second.process_rcvd_removed(
                [&](base::Id mId) {
                    typename TTypes::TerminalRemovedAck msg;
                    msg[fields::terminalId] = mId;

                    connection.send(msg);
                },
                [&](bool) {
                    stillOwned = false;
                }
            );

            if (stillOwned) {
                return;
            }
        }

        // remove the owner
        owners.erase(ownerIt);
        on_terminal_owner_removed(connection, tm);

        // store the binding Id for later
        auto bindingId = tm->binding;

        // process the FSM for terminal related stuff
        switch (tm->state) {
        case STATE_OWNERS_1_LEAFS_0_NODES:
        case STATE_OWNERS_0_LEAFS_1_NODES:
            m_terminals.hide(tm);
            disassociate(tm);
            update_users_for_abandoned_terminal(tm, &connection);
            break;

        case STATE_OWNERS_M_LEAFS_0_NODES:
        case STATE_OWNERS_N_LEAFS_N_NODES:
        case STATE_OWNERS_0_LEAFS_M_NODES:
            break;

        case STATE_AWAIT_ACKS:
        case STATE_JUST_CREATED:
            YOGI_NEVER_REACHED;
        }

        // update the state of the FSM
        bool alive = update_state(tm);

        if (!alive) {
            if (bindingId) {
                auto& bd = m_bindings[bindingId];
                for (auto& owner : bd.owningLeafs) {
                    if (owner.first != &connection) {
                        typename TTypes::BindingReleased msg;
                        msg[fields::bindingId] = owner.second.mapped_id();
                        owner.first->send(msg);
                    }
                }
            }

            return;
        }

        // if there is now only one owning node left, we tell that node
        // that we do not own the terminal any more
        if (tm->state == STATE_OWNERS_0_LEAFS_1_NODES) {
            auto user = tm->usingNodes.find(tm->owningNodes.begin()->first);

            if (!user->second.destroyed()) {
                bool alive = user->second.process_object_destroyed(
                    [&](base::Id mappedId) {
                        typename TTypes::TerminalRemoved msg;
                        msg[fields::mappedId] = mappedId;

                        user->first->send(msg);
                    }
                );

                // TODO: I believe, the alive check is not necessary because the
                // FSM can never be dead here. This would always apply when
                // calling the process_object_destroyed() function
                YOGI_ASSERT(alive);
                if (!alive) {
                    tm->usingNodes.erase(user);
                    on_terminal_user_removed(connection, tm);
                }
            }
        }

        // update the state of the bindings
        if (bindingId) {
            auto& bd = m_bindings[bindingId];

            if (tm->state == STATE_OWNERS_1_LEAFS_0_NODES) {
                auto owner = bd.owningLeafs.find(
                    tm->owningLeafs.begin()->first);
                if (owner != bd.owningLeafs.end()) {
                    typename TTypes::BindingReleased msg;
                    msg[fields::bindingId] = owner->second.mapped_id();

                    owner->first->send(msg);
                }
            }
            else if (tm->state == STATE_AWAIT_ACKS) {
                for (auto& owner : bd.owningLeafs) {
                    if (owner.first != &connection) {
                        typename TTypes::BindingReleased msg;
                        msg[fields::bindingId] = owner.second.mapped_id();

                        owner.first->send(msg);
                    }
                }
            }
        }

        // if the terminal does not have any owners any more, we remove the
        // association with the binding
        if (tm->hidden()) {
            disassociate(tm);
        }
    }

protected:
    explicit NodeLogicBaseT(interfaces::IScheduler& scheduler,
        known_terminals_changed_fn knownTerminalsChangedFn)
        : m_scheduler{scheduler.make_ptr<interfaces::IScheduler>()}
        , m_knownTerminalsChangedFn{knownTerminalsChangedFn}
    {
        add_msg_handler<typename TTypes::TerminalDescription>(this,
            &NodeLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalMapping>(this,
            &NodeLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalRemoved>(this,
            &NodeLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::TerminalRemovedAck>(this,
            &NodeLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingDescription>(this,
            &NodeLogicBaseT::on_message_received);
        add_msg_handler<typename TTypes::BindingRemoved>(this,
            &NodeLogicBaseT::on_message_received);
    }

    template <typename TMsg, typename TTgt, typename TO>
    void add_msg_handler(TTgt* tgt, void (TO::*fn)(TMsg&&,
        interfaces::IConnection&))
    {
        auto msgTypeIdNum = TMsg{}.type_id().number();
        if (m_msgHandlers.size() <= msgTypeIdNum) {
            m_msgHandlers.resize(msgTypeIdNum + 1);
        }

        YOGI_ASSERT(!m_msgHandlers[msgTypeIdNum]);

        m_msgHandlers[msgTypeIdNum] = [=](interfaces::IMessage& msg,
            interfaces::IConnection& origin) {
            auto lock = make_lock_guard();
            (tgt->*fn)(std::move(static_cast<TMsg&>(msg)), origin);
        };
    }

    std::unique_lock<std::mutex> make_lock_guard()
    {
        return std::unique_lock<std::mutex>{m_mutex};
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

    bool is_connection_alive(interfaces::IConnection& connection)
    {
        if (connection.remote_is_node()) {
            return m_nodeConnections.count(&connection) != 0;
        }
        else {
            return m_leafConnections.count(&connection) != 0;
        }
    }

    void on_connection_started(interfaces::IConnection& connection)
    {
		using namespace messaging;

        auto lock = make_lock_guard();

        if (connection.remote_is_node()) {
            // tell the node what we've got
            for (auto tm = m_terminals.begin(); tm != m_terminals.end(); ++tm) {
                if (!tm->hidden()) {
                    YOGI_ASSERT(!tm->usingNodes.count(&connection));
                    tm->usingNodes[&connection].reset(false, [&] {
                        typename TTypes::TerminalDescription msg;
                        prepare_message(&msg, tm);
                        msg[fields::identifier] = tm->identifier();
                        msg[fields::id]         = tm->id();

                        connection.send(msg);
                    });

                    on_terminal_user_added(connection, tm);
                }
            }

            // store the connection
            YOGI_ASSERT(m_nodeConnections.count(&connection) == 0);
            m_nodeConnections.emplace(&connection);
        }
        else {
            // store the connection
            YOGI_ASSERT(m_leafConnections.count(&connection) == 0);
            m_leafConnections.emplace(&connection);
        }
    }

    void on_connection_destroyed(interfaces::IConnection& connection)
    {
        auto lock = make_lock_guard();

        // remove the connection
        try {
            auto& connSet = connection.remote_is_node() ? m_nodeConnections
                : m_leafConnections;
            if (!connSet.erase(&connection)) {
                return;
            }
        }
        catch (const api::ExceptionT<YOGI_ERR_NOT_READY>&) {
            // on_connection_started() has not been called
            return;
        }

        // remove binding owners
        auto bd = m_bindings.begin();
        while (bd != m_bindings.end()) {
            if (bd->owningLeafs.erase(&connection)) {
                on_binding_owner_removed(connection, bd);
            }

            if (bd->owningLeafs.empty()) {
                disassociate(bd);
                bd = m_bindings.erase(bd);
            }
            else {
                ++bd;
            }
        }

        // remove terminal owners and users and inform other nodes
        auto tm = m_terminals.begin();
        while (tm != m_terminals.end()) {
            auto tmpTm = tm;
            ++tmpTm;

            tm->usingNodes.erase(&connection);
            on_terminal_user_removed(connection, tm);

            if (update_state(tm)) { // TODO: is update_state() needed here?
                remove_terminal_owner(tm, connection, true);
            }

            tm = tmpTm;
        }
    }

    virtual void on_terminal_owner_added(interfaces::IConnection& connection,
        const_terminal_iterator tm,
        const typename TTypes::TerminalDescription& msg)
    {
    }

    virtual void on_terminal_owner_removed(interfaces::IConnection& connection,
        const_terminal_iterator tm)
    {
    }

    virtual void on_terminal_owner_remapped(interfaces::IConnection& connection,
        const_terminal_iterator tm, base::Id newMappedId)
    {
    }

    virtual void on_terminal_user_added(interfaces::IConnection& connection,
        const_terminal_iterator tm)
    {
    }

    virtual void on_terminal_user_removed(interfaces::IConnection& connection,
        const_terminal_iterator tm)
    {
    }

    virtual void on_binding_owner_added(interfaces::IConnection& connection,
        const_binding_iterator bd, base::Id mappedId)
    {
    }

    virtual void on_binding_owner_removed(interfaces::IConnection& connection,
        const_binding_iterator bd)
    {
    }

    virtual void on_binding_owner_remapped(interfaces::IConnection& connection,
        const_binding_iterator bd, base::Id newMappedId)
    {
    }

    virtual void prepare_message(typename TTypes::TerminalDescription* msg,
        const_terminal_iterator tm)
    {
    }

public:
    std::vector<base::Identifier> get_known_terminals()
    {
        auto lock = make_lock_guard();

        std::vector<base::Identifier> terminals;
        for (auto& tm : m_terminals) {
            if (!tm.identifier().hidden()) {
                terminals.push_back(tm.identifier());
            }
        }

        return terminals;
    }

    void on_message_received(typename TTypes::TerminalDescription&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        base::Id originTerminalId = msg[fields::id];

        // find/create the terminal info entry
        auto tm = m_terminals.insert(msg[fields::identifier]).first;
        bool terminalAdded = tm->state == STATE_JUST_CREATED;

        auto res = ownersMap(tm, origin).insert(std::make_pair(&origin,
            MappedObjectFsm{}));

        auto& ownerFsm = res.first->second;

        // map the terminal and add the connection as an owner; if we already
        // know the owner, we don't have to do anything else
        base::Id oldMappedId;
        ownerFsm.process_rcvd_description(msg[fields::id],
            [&] {
                typename TTypes::TerminalMapping reply;
                reply[fields::terminalId] = msg[fields::id];
                reply[fields::mappedId]   = tm->id();

                origin.send(reply);
            },
            [&](base::Id mId) {
                oldMappedId = mId;

                typename TTypes::TerminalRemovedAck reply;
                reply[fields::terminalId] = mId;

                origin.send(reply);
            },
            [](bool) {
            }
        );

        if (oldMappedId) {
            on_terminal_owner_remapped(origin, tm, ownerFsm.mapped_id());
            return;
        }

        // process the FSM for terminal related stuff
        switch (tm->state) {
        case STATE_JUST_CREATED:
            // tell all connected nodes about the new terminal
            msg[fields::id] = tm->id();
            for (auto conn : m_nodeConnections) {
                if (conn != &origin) {
                    tm->usingNodes[conn].reset(false, [&] {conn->send(msg); });
                }
            }

            msg[fields::id] = originTerminalId;
            break;

        case STATE_OWNERS_1_LEAFS_0_NODES:
        case STATE_OWNERS_M_LEAFS_0_NODES:
        case STATE_OWNERS_0_LEAFS_M_NODES:
        case STATE_OWNERS_N_LEAFS_N_NODES:
            break;

        case STATE_OWNERS_0_LEAFS_1_NODES: {{
            // tell the owning node that we now own the terminal too
            msg[fields::id] = tm->id();

            interfaces::IConnection* owner = std::find_if(
                tm->owningNodes.begin(), tm->owningNodes.end(),
                [&](const fsm_map::value_type& val) {
                    return val.first != &origin;
                }
            )->first;

            if (!tm->usingNodes.count(owner)) {
                tm->usingNodes[owner].reset(false, [&] {owner->send(msg);});
            }

            msg[fields::id] = originTerminalId;
            }} break;

        case STATE_AWAIT_ACKS:
            YOGI_NEVER_REACHED;
        }

        // try to associate the terminal with a binding
        auto bd = associate(tm);

        // update the state of the bindings
        if (bd != m_bindings.end()) {
            if (tm->state == STATE_JUST_CREATED) {
                for (auto& owner : bd->owningLeafs) {
                    if (owner.first != &origin) {
                        typename TTypes::BindingEstablished msg;
                        msg[fields::bindingId] = owner.second.mapped_id();

                        owner.first->send(msg);
                    }
                }
            }
            else if (tm->state == STATE_OWNERS_1_LEAFS_0_NODES) {
                for (auto& epOwner : tm->owningLeafs) {
                    if (epOwner.first != &origin) {
                        auto bdOwner = bd->owningLeafs.find(epOwner.first);
                        if (bdOwner != bd->owningLeafs.end()) {
                            typename TTypes::BindingEstablished msg;
                            msg[fields::bindingId] = bdOwner->second.mapped_id();

                            bdOwner->first->send(msg);
                        }
                        break;
                    }
                }
            }
        }

        // update the state of the FSM
        update_state(tm);

        // inform derived classes that we added a new terminal owner
        if (res.second) {
            on_terminal_owner_added(origin, tm, msg);
        }

        // inform library user that we now know a new terminal
        if (terminalAdded && !tm->identifier().hidden()) {
            m_knownTerminalsChangedFn(tm->identifier(), true);
        }
    }

    void on_message_received(typename TTypes::TerminalMapping&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        // we can only receive this message from a node
        YOGI_ASSERT(origin.remote_is_node());

        // find the endpoint info entry
        auto tm = m_terminals.find(msg[fields::terminalId]);

        // process the FSM
        switch (tm->state) {
        case STATE_JUST_CREATED:
            YOGI_NEVER_REACHED;

        case STATE_OWNERS_1_LEAFS_0_NODES:
        case STATE_OWNERS_M_LEAFS_0_NODES:
        case STATE_OWNERS_0_LEAFS_M_NODES:
        case STATE_OWNERS_N_LEAFS_N_NODES:
        case STATE_OWNERS_0_LEAFS_1_NODES:
        case STATE_AWAIT_ACKS:
            // store the received mapped ID
            tm->usingNodes[&origin].process_rcvd_mapping(msg[fields::mappedId],
                [&] {
                    typename TTypes::TerminalRemoved reply;
                    reply[fields::mappedId] = msg[fields::mappedId];

                    origin.send(reply);
                },
                [&](base::Id) {
                    YOGI_NEVER_REACHED;
                },
                [&](bool) {
                }
            );
            break;
        }
    }

    void on_message_received(typename TTypes::TerminalRemoved&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        auto tm = m_terminals.find(msg[fields::mappedId]);
        remove_terminal_owner(tm, origin, false);
    }

    void on_message_received(typename TTypes::TerminalRemovedAck&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        // we can only receive this message from a node
        YOGI_ASSERT(origin.remote_is_node());

        // find the terminal info entry and the user entry
        auto tm   = m_terminals.find(msg[fields::terminalId]);
        auto user = tm->usingNodes.find(&origin);

        // process the FSM
        switch (tm->state) {
        case STATE_JUST_CREATED:
            YOGI_NEVER_REACHED;

        case STATE_OWNERS_0_LEAFS_M_NODES:
        case STATE_OWNERS_1_LEAFS_0_NODES:
        case STATE_OWNERS_N_LEAFS_N_NODES:
        case STATE_OWNERS_M_LEAFS_0_NODES: {{
            typename TTypes::TerminalDescription reply;
            prepare_message(&reply, tm);
            reply[fields::identifier] = tm->identifier();
            reply[fields::id]         = tm->id();

            user->second.reset(false, [&] { origin.send(reply); });
            }}
            break;

        case STATE_OWNERS_0_LEAFS_1_NODES:
            if (!tm->owningNodes.count(&origin)) {
                typename TTypes::TerminalDescription reply;
                prepare_message(&reply, tm);
                reply[fields::identifier] = tm->identifier();
                reply[fields::id]         = tm->id();

                user->second.reset(false, [&] { origin.send(reply); });
                break;
            }
            // intentional fallthrough

        case STATE_AWAIT_ACKS:
            if (!user->second.process_rcvd_removed_ack()) {
                tm->usingNodes.erase(user);
                on_terminal_user_removed(origin, tm);
            }
            break;
        }

        // if there are no more owner and no more users, we remove the
        // association with the binding
        if (tm->owningLeafs.empty() && tm->owningNodes.empty() &&
            tm->usingNodes.empty()) {
                disassociate(tm);
        } // TODO: this is redundant because in update_state()

        // update the FSM state or remove the terminal
        update_state(tm);
    }

    void on_message_received(typename TTypes::BindingDescription&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        // we can only receive this message from a leaf
        YOGI_ASSERT(!origin.remote_is_node());

        // create/get the binding and the owner entry
        auto bd        = m_bindings.insert(msg[fields::identifier]).first;
        auto& ownerFsm = bd->owningLeafs[&origin];

        // map the binding and add the connection as an owner; if we already
        // know the owner, we don't have to do anything else
        base::Id oldMappedId;
        ownerFsm.process_rcvd_description(msg[fields::id],
            [&] {
                typename TTypes::BindingMapping reply;
                reply[fields::bindingId] = msg[fields::id];
                reply[fields::mappedId]  = bd->id();

                origin.send(reply);
            },
            [&](base::Id mId) {
                oldMappedId = mId;

                typename TTypes::BindingRemovedAck reply;
                reply[fields::bindingId] = mId;

                origin.send(reply);
            },
            [](bool) {}
        );

        if (oldMappedId) {
            on_binding_owner_remapped(origin, bd, ownerFsm.mapped_id());
            return;
        }

        // try to associate the binding with a terminal
        auto tm = associate(bd);

        // check if the binding is established
        if (tm != m_terminals.end()) {
            if (tm->state != STATE_OWNERS_1_LEAFS_0_NODES
                || tm->owningLeafs.begin()->first != &origin) {
                    typename TTypes::BindingEstablished reply;
                    reply[fields::bindingId] = msg[fields::id];

                    origin.send(reply);
            }
        }

        // inform derived classes that we added a new binding owner
        on_binding_owner_added(origin, bd, ownerFsm.mapped_id());
    }

    void on_message_received(typename TTypes::BindingRemoved&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        // we can only receive this message from a leaf
        YOGI_ASSERT(!origin.remote_is_node());

        // find the corresponding binding info entry and the owner entry
        auto bd    = m_bindings.find(msg[fields::mappedId]);
        auto owner = bd->owningLeafs.find(&origin);

        // update the FSM
        owner->second.process_rcvd_removed(
            [&](base::Id mId) {
                typename TTypes::BindingRemovedAck reply;
                reply[fields::bindingId] = mId;

                origin.send(reply);
            },
            [](bool) {}
        );

        // if the binding is still mapped, we do nothing else
        if (owner->second.is_mapped()) {
            return;
        }

        // remove the owner
        bd->owningLeafs.erase(owner);
        on_binding_owner_removed(origin, bd);

        // if there are no more binding owners, we remove the binding
        if (bd->owningLeafs.empty()) {
            disassociate(bd);
            m_bindings.erase(bd);
        }
    }
};

} // namespace common
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_COMMON_NODELOGICBASET_HPP
