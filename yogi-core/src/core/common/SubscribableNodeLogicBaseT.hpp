#ifndef CHIRP_CORE_COMMON_SUBSCRIBABLENODELOGICBASET_HPP
#define CHIRP_CORE_COMMON_SUBSCRIBABLENODELOGICBASET_HPP

#include "../../config.h"
#include "NodeLogicBaseT.hpp"
#include "subscribable_logic_types.hpp"


namespace chirp {
namespace core {
namespace common {

/***************************************************************************//**
 * Templated base class for node logic implementations that support
 * subscriptions
 ******************************************************************************/
template <typename TTypes>
class SubscribableNodeLogicBaseT : public NodeLogicBaseT<TTypes>
{
protected:
    typedef NodeLogicBaseT<TTypes> super;
    typedef typename super::terminal_info terminal_info;
    typedef typename super::binding_info binding_info;
    typedef typename super::known_terminals_changed_fn known_terminals_changed_fn;
    typedef typename super::fsm_map fsm_map;

private:
    template <typename TFn>
    void foreach_mapped_owner(const terminal_info& tm, TFn fn,
        const interfaces::IConnection* exception = nullptr)
    {
        for (auto owners : {&tm.owningLeafs, &tm.owningNodes}) {
            for (auto& owner : *owners) {
                if (owner.first != exception && owner.second.is_mapped()) {
                    fn(owner);
                }
            }
        }
    }

    interfaces::IConnection* only_subscriber_or_binding_owner(
        const terminal_info& tm)
    {
        auto& subs = tm.subscribers;

        if (subs.size() > 1) {
            return nullptr;
        }

        if (tm.binding) {
            auto& owners = super::get_binding_info(tm.binding).owningLeafs;
            if (owners.size() == 1) {
                interfaces::IConnection* onlyOwner = owners.begin()->first;
                if (subs.empty() || subs.begin()->first == onlyOwner) {
                    return onlyOwner;
                }
                else {
                    return nullptr;
                }
            }
            else if (owners.size() > 1) {
                return nullptr;
            }
        }

        return subs.empty() ? nullptr : subs.begin()->first;
    }

    template <typename TMap>
    bool contains_entries_other_than(const TMap& map,
        interfaces::IConnection& exception)
    {
        switch (map.size()) {
        case 0:
            return false;

        case 1:
            return map.begin()->first != &exception;

        default:
            return true;
        }
    }

    bool at_least_one_subscriber_or_binding_owner_besides(
        const terminal_info& tm, interfaces::IConnection& connection)
    {
        CHIRP_ASSERT(!tm.binding || !super::get_binding_info(tm.binding)
            .owningLeafs.empty());

        // another subscribers besides connection?
        if (contains_entries_other_than(tm.subscribers, connection)) {
            return true;
        }

        // any binding owners besides connection?
        if (tm.binding) {
            auto& owners = super::get_binding_info(tm.binding).owningLeafs;
            if (contains_entries_other_than(owners, connection)) {
                return true;
            }
        }

        return false;
    }

    interfaces::IConnection* second_binding_owner_of_two(
        const binding_info& bd, interfaces::IConnection& firstOwner)
    {
        if (bd.owningLeafs.size() == 2) {
            auto bdOwner = bd.owningLeafs.begin();
            if (bdOwner->first == &firstOwner) {
                bdOwner = ++bdOwner;
            }

            return bdOwner->first;
        }
        
        return nullptr;
    }

    bool no_other_subscriber_or_binding_owner(const terminal_info& tm,
        interfaces::IConnection& connection)
    {
        CHIRP_ASSERT(!tm.binding || !super::get_binding_info(
            tm.binding).owningLeafs.empty());

        if (tm.subscribers.empty() && (!tm.binding)) {
            return true;
        }
        else {
            return only_subscriber_or_binding_owner(tm) == &connection;
        }
    }

    bool no_subscribers_or_binding_owners(const terminal_info& tm)
    {
        return tm.subscribers.empty() && (!tm.binding
            || super::get_binding_info(tm.binding).owningLeafs.empty());
    }

    base::Id terminal_owner_fsm_mapped_id(const binding_info& bd,
        interfaces::IConnection& connection)
    {
        if (!bd.terminal) {
            return base::Id{};
        }

        auto& tm = super::get_terminal_info(bd.terminal);
        return terminal_owner_fsm_mapped_id(tm, connection);
    }

    base::Id terminal_owner_fsm_mapped_id(const terminal_info& tm,
        interfaces::IConnection& connection)
    {
        if (connection.remote_is_node()) {
            auto owner = tm.owningNodes.find(&connection);
            if (owner == tm.owningNodes.end()) {
                return base::Id{};
            }
            else {
                return owner->second.mapped_id();
            }
        }
        else {
            auto owner = tm.owningLeafs.find(&connection);
            if (owner == tm.owningLeafs.end()) {
                return base::Id{};
            }
            else {
                return owner->second.mapped_id();
            }
        }
    }

    void unsubscribe_if_needed(const terminal_info& tm,
        interfaces::IConnection& exception)
    {
		using namespace messaging;

		typename TTypes::Unsubscribe msg;
        auto onlyConn = only_subscriber_or_binding_owner(tm);
        CHIRP_ASSERT(onlyConn != &exception);
        if (onlyConn) {
            base::Id id = terminal_owner_fsm_mapped_id(tm, *onlyConn);
            if (id) {
				msg[fields::terminalId] = id;
                onlyConn->send(msg);
            }
        }
        else if (no_subscribers_or_binding_owners(tm)) {
            foreach_mapped_owner(tm, [&](const typename fsm_map::value_type& own) {
				msg[fields::terminalId] = own.second.mapped_id();
                own.first->send(msg);
            }, &exception);
        }
    }

    void remove_subscriber_if_needed(interfaces::IConnection& connection,
        const terminal_info& tm)
    {
        if (tm.subscribers.erase(&connection)) {
            unsubscribe_if_needed(tm, connection);
            on_subscriber_removed(connection, tm);
        }
    }

    virtual void on_subscriber_removed(interfaces::IConnection& connection,
        const terminal_info& tm)
    {
    }

protected:
    SubscribableNodeLogicBaseT(interfaces::IScheduler& scheduler,
        known_terminals_changed_fn knownTerminalsChangedFn)
        : super{scheduler, knownTerminalsChangedFn}
    {
        super::template add_msg_handler<typename TTypes::Subscribe>(this,
            &SubscribableNodeLogicBaseT::on_message_received);
        super::template add_msg_handler<typename TTypes::Unsubscribe>(this,
            &SubscribableNodeLogicBaseT::on_message_received);
    }
    
    virtual void on_terminal_owner_added(interfaces::IConnection& connection,
        typename super::const_terminal_iterator tm,
        const typename TTypes::TerminalDescription& msg) override
    {
		using namespace messaging;

        CHIRP_ASSERT(tm->owningLeafs.count(&connection)
            + tm->owningNodes.count(&connection) == 1);
        CHIRP_ASSERT(!tm->owningLeafs.count(&connection)
            || tm->owningLeafs.find(&connection)->second.is_mapped());
        CHIRP_ASSERT(!tm->owningNodes.count(&connection)
            || tm->owningNodes.find(&connection)->second.is_mapped());
        
        if (at_least_one_subscriber_or_binding_owner_besides(*tm, connection)) {
            typename TTypes::Subscribe fwMsg;
			fwMsg[fields::terminalId] = msg[fields::id];
            connection.send(fwMsg);
        }
    }

    virtual void on_terminal_owner_remapped(interfaces::IConnection& connection,
        typename super::const_terminal_iterator tm, base::Id newMappedId)
        override
    {
		using namespace messaging;

        CHIRP_ASSERT(!tm->owningLeafs.count(&connection)
            || tm->owningLeafs.find(&connection)->second.is_mapped());
        CHIRP_ASSERT(!tm->owningNodes.count(&connection)
            || tm->owningNodes.find(&connection)->second.is_mapped());

        if (at_least_one_subscriber_or_binding_owner_besides(*tm, connection)) {
            typename TTypes::Subscribe msg;
			msg[fields::terminalId] = newMappedId;
            connection.send(msg);
        }
    }

    virtual void on_terminal_owner_removed(interfaces::IConnection& connection,
        typename super::const_terminal_iterator tm) override
    {
        if (!tm->owningLeafs.empty()) {
            return;
        }

        // if there are no owners then there are no subscribers
        if (tm->owningNodes.empty()) {
            tm->subscribers.clear();
        }
        // if there is only one owning node left, this node cannot be subscribed
        else if (tm->owningNodes.size() == 1) {
            auto ownIt = tm->owningNodes.begin();
            auto subIt = tm->subscribers.find(ownIt->first);
            if (subIt != tm->subscribers.end()) {
                //CHIRP_ASSERT(subIt->second == ownIt->second.mapped_id()); // TODO: remove (?) only if no fundamental problem
                tm->subscribers.erase(subIt);
            }
        }
    }

    virtual void on_terminal_user_removed(interfaces::IConnection& connection,
        typename super::const_terminal_iterator tm) override
    {
        CHIRP_ASSERT(!tm->usingNodes.count(&connection));

        remove_subscriber_if_needed(connection, *tm);
    }

    virtual void on_binding_owner_added(interfaces::IConnection& connection,
        typename super::const_binding_iterator bd, base::Id mappedId) override
    {
		using namespace messaging;

        CHIRP_ASSERT(bd->owningLeafs.count(&connection));

        if (!bd->terminal) {
            return;
        }

        auto& tm = super::get_terminal_info(bd->terminal);

        if (no_other_subscriber_or_binding_owner(tm, connection)) {
            typename TTypes::Subscribe msg;
            foreach_mapped_owner(tm, [&](const typename fsm_map::value_type& own) {
				msg[fields::terminalId] = own.second.mapped_id();
                own.first->send(msg);
            }, &connection);
        }
        else if (tm.subscribers.size() == 1 && bd->owningLeafs.size() == 1){
            auto subIt  = tm.subscribers.begin();
            auto ownIt = tm.owningNodes.find(subIt->first);
            if (ownIt != tm.owningNodes.end()) {
                CHIRP_ASSERT(ownIt->second.is_mapped());
                base::Id ownId = ownIt->second.mapped_id();
                typename TTypes::Subscribe msg;
				msg[fields::terminalId] = ownId;
                subIt->first->send(msg);
            }
        }
        else if (tm.subscribers.empty() && bd->owningLeafs.size() == 2) {
            auto otherOwner = second_binding_owner_of_two(*bd, connection);
            CHIRP_ASSERT(otherOwner);

            base::Id id = terminal_owner_fsm_mapped_id(*bd, *otherOwner);
            if (id) {
                typename TTypes::Subscribe msg;
				msg[fields::terminalId] = id;
                otherOwner->send(msg);
            }
        }
    }

    virtual void on_binding_owner_removed(interfaces::IConnection& connection,
        typename super::const_binding_iterator bd) override
    {
        CHIRP_ASSERT(!connection.remote_is_node());
        CHIRP_ASSERT(!bd->owningLeafs.count(&connection));

        if (!bd->terminal) {
            return;
        }

        auto& tm = super::get_terminal_info(bd->terminal);
        unsubscribe_if_needed(tm, connection);
        on_subscriber_removed(connection, tm);
    }

    void on_message_received(typename TTypes::Subscribe&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        CHIRP_ASSERT(origin.remote_is_node());
        
        auto& tm = super::get_terminal_info(msg[fields::terminalId]);

        CHIRP_ASSERT(!tm.subscribers.count(&origin));
        CHIRP_ASSERT(tm.usingNodes.count(&origin));

        auto subsFsm = tm.usingNodes.find(&origin)->second;
        if (!subsFsm.is_mapped()) {
            return;
        }

        typename TTypes::Subscribe fwMsg;
        auto otherConn = only_subscriber_or_binding_owner(tm);
        if (otherConn && otherConn != &origin) {
            base::Id id = terminal_owner_fsm_mapped_id(tm, *otherConn);
            if (id) {
				fwMsg[fields::terminalId] = id;
                otherConn->send(fwMsg);
            }
        }
        else if (tm.subscribers.empty() && !tm.binding) {
            foreach_mapped_owner(tm, [&](const typename fsm_map::value_type& own) {
				fwMsg[fields::terminalId] = own.second.mapped_id();
                own.first->send(fwMsg);
            }, &origin);
        }

        tm.subscribers[&origin] = subsFsm.mapped_id();

        on_subscribed(origin, subsFsm.mapped_id(), tm);
    }

    void on_message_received(typename TTypes::Unsubscribe&& msg,
        interfaces::IConnection& origin)
    {
		using namespace messaging;

        auto& tm = super::get_terminal_info(msg[fields::terminalId]);
        remove_subscriber_if_needed(origin, tm);
    }

    virtual void on_subscribed(interfaces::IConnection& origin,
        base::Id subscriptionId, const typename super::terminal_info& tm)
    {
    }
};

} // namespace common
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_COMMON_SUBSCRIBABLENODELOGICBASET_HPP
