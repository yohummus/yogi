#ifndef CHIRP_CORE_SCATTER_GATHER_LEAFLOGIC_HPP
#define CHIRP_CORE_SCATTER_GATHER_LEAFLOGIC_HPP

#include "../../config.h"
#include "../common/SubscribableLeafLogicBaseT.hpp"
#include "Terminal.hpp"
#include "logic_types.hpp"


namespace chirp {
namespace core {
namespace scatter_gather {

/***************************************************************************//**
 * Implements the logic for scatter-gather terminals on leafs
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class LeafLogic : public common::SubscribableLeafLogicBaseT<TTypes>
{
public:
    typedef common::SubscribableLeafLogicBaseT<TTypes> super;
    typedef typename TTypes::terminal_type terminal_type;

private:
    base::ObjectRegister<terminal_type*>              m_scatterOperations;
    std::unordered_multimap<base::Id, terminal_type*> m_gatherOperations;

private:
    template <typename TMultiMap>
    void erase_from_multimap(TMultiMap* map,
        const typename TMultiMap::key_type& key,
        const typename TMultiMap::mapped_type& value)
    {
        CHIRP_ASSERT(map->count(key));

        auto range = map->equal_range(key);
        auto it = std::find_if(range.first, range.second,
            [&](const typename TMultiMap::value_type& entry) {
                return entry.second == value;
            }
        );

        CHIRP_ASSERT(it != range.second);
        map->erase(it);
    }

    void remove_all_gather_operations(const typename super::terminal_info& tm)
    {
        for (auto& entry : tm.ext.gatherOperations) {
            auto& bd = super::get_binding_info(entry.second);
            erase_from_multimap(&m_gatherOperations, entry.first, tm.terminal);

            CHIRP_ASSERT(bd.ext.gatherOperations.count(entry.first));
            erase_from_multimap(&bd.ext.gatherOperations, entry.first, 
                tm.terminal->id());
        }
    }

protected:
    LeafLogic(interfaces::IScheduler& scheduler)
        : super{scheduler}
    {
        super::template add_msg_handler<typename TTypes::Scatter>(this,
            &LeafLogic::on_message_received);
        super::template add_msg_handler<typename TTypes::Gather>(this,
            &LeafLogic::on_message_received);
    }

    void on_message_received(typename TTypes::Scatter&& msg)
    {
        using namespace messaging;

        auto& bd = super::get_binding_info(msg[fields::subscriptionId]);

        //CHIRP_ASSERT(bd.established || bd.fsm.destroyed()); TODO
        CHIRP_ASSERT(!m_gatherOperations.count(msg[fields::operationId]));
        CHIRP_ASSERT(!bd.ext.gatherOperations.count(msg[fields::operationId]));

        if (!bd.established) {
            return;
        }

        for (auto& binding : bd.bindings) {
            auto& tm = super::get_terminal_info(binding->terminal().id());

            // store gather operation in leaf
            m_gatherOperations.insert(std::make_pair(msg[fields::operationId], 
                tm.terminal));

            // store gather operation in binding group
            bd.ext.gatherOperations.insert(std::make_pair(
				msg[fields::operationId], tm.terminal->id()));

            // store gather operation in terminal
            tm.ext.gatherOperations.insert(std::make_pair(
				msg[fields::operationId], msg[fields::subscriptionId]));

            // let terminal notify the library user
            tm.terminal->on_scattered_message_received(msg[fields::operationId],
                std::move(msg[fields::data]));
        }
    }

    void on_message_received(typename TTypes::Gather&& msg)
    {
		using namespace messaging;

        auto terminal = m_scatterOperations[msg[fields::operationId]];

        if (terminal) {
            bool abort = !terminal->on_gathered_message_received(
                msg[fields::operationId], msg[fields::gatherFlags],
				std::move(msg[fields::data]));

            if (abort) {
                auto& tm = super::get_terminal_info(terminal->id());

                CHIRP_ASSERT(tm.ext.scatterOperations.count(
					msg[fields::operationId]));
                tm.ext.scatterOperations.erase(msg[fields::operationId]);

                CHIRP_ASSERT(m_scatterOperations.count(
					msg[fields::operationId]));
                m_scatterOperations[msg[fields::operationId]] = nullptr;

                terminal = nullptr;
            }
        }

        if (msg[fields::gatherFlags] & gather_flags::GATHER_FINISHED) {
            if (terminal) {
                auto& tm = super::get_terminal_info(terminal->id());

                CHIRP_ASSERT(tm.ext.scatterOperations.count(
					msg[fields::operationId]));
                tm.ext.scatterOperations.erase(msg[fields::operationId]);
            }
            
            m_scatterOperations.erase(msg[fields::operationId]);
        }
    }

    virtual void on_terminal_removed(typename TTypes::terminal_type& terminal,
        const typename super::terminal_info& tm) override
    {
        // cleanup scatter operations
        for (auto& operationId : tm.ext.scatterOperations) {
            CHIRP_ASSERT(m_scatterOperations.count(operationId));
            m_scatterOperations[operationId] = nullptr;
        }

        // cleanup gather operations
        remove_all_gather_operations(tm);
    }


    virtual void on_terminal_mapping_changed(bool isMapped,
        const typename super::terminal_info& tm) override
    {
        super::on_terminal_mapping_changed(isMapped, tm);

        if (!tm.ext.scatterOperations.empty()) {
            CHIRP_ASSERT(!super::connected() || !isMapped);

            gather_flags flags = super::connected() ? GATHER_BINDINGDESTROYED :
                GATHER_CONNECTIONLOST;

            for (auto& operationId : tm.ext.scatterOperations) {
                tm.terminal->on_gathered_message_received(operationId,
                    flags | GATHER_FINISHED, base::Buffer{});

                CHIRP_ASSERT(m_scatterOperations.count(operationId));
                m_scatterOperations.erase(operationId);
            }

            tm.ext.scatterOperations.clear();
        }

        if (!tm.ext.gatherOperations.empty()) {
            remove_all_gather_operations(tm);
            tm.ext.gatherOperations.clear();
        }
    }

    virtual void on_binding_group_mapping_changed(bool isMapped,
        const typename super::binding_info& bd) override
    {
        super::on_binding_group_mapping_changed(isMapped, bd);

        if (!bd.ext.gatherOperations.empty()) {
            CHIRP_ASSERT(!super::connected() || !isMapped);

            for (auto& entry : bd.ext.gatherOperations) {
                auto& tm = super::get_terminal_info(entry.second);
                CHIRP_ASSERT(tm.ext.gatherOperations.count(entry.first));
                tm.ext.gatherOperations.erase(entry.first);

                erase_from_multimap(&m_gatherOperations, entry.first,
                    tm.terminal);
            }

            bd.ext.gatherOperations.clear();
        }
    }

    virtual void on_binding_removed(interfaces::IBinding& binding,
        const typename super::binding_info& bd) override
    {
		using namespace messaging;

        auto it = std::begin(bd.ext.gatherOperations);
        while (it != std::end(bd.ext.gatherOperations)) {
            auto& tm = super::get_terminal_info(it->second);
            CHIRP_ASSERT(tm.ext.gatherOperations.count(it->first));

            if (tm.terminal->id() == binding.terminal().id()) {
                tm.ext.gatherOperations.erase(it->first);

                erase_from_multimap(&m_gatherOperations, it->first,
                    tm.terminal);

                if (!m_gatherOperations.count(it->first)) {
                    typename TTypes::Gather msg;
                    msg[fields::operationId] = it->first;
                    msg[fields::gatherFlags] = GATHER_BINDINGDESTROYED
						| GATHER_FINISHED;
                    msg[fields::data] = base::Buffer{};

                    super::connection().send(msg);
                }

                it = bd.ext.gatherOperations.erase(it);
            }
            else {
                ++it;
            }
        }
    }

public:
    virtual std::pair<base::Id, std::unique_lock<std::recursive_mutex>>
        sg_scatter(typename TTypes::terminal_type& terminal,
            base::Buffer&& data)
    {
		using namespace messaging;

        auto lock = super::make_lock_guard();

        auto& info = super::get_terminal_info(terminal.id());
        if (!info.subscribed) {
            throw api::ExceptionT<CHIRP_ERR_NOT_BOUND>{};
        }

        base::Id id;

        try {
            id = m_scatterOperations.insert(&terminal);

            CHIRP_ASSERT(!info.ext.scatterOperations.count(id));
            info.ext.scatterOperations.insert(id);
        }
        catch (...) {
            m_scatterOperations.erase(id);
            throw;
        }


        typename TTypes::Scatter msg;
        msg[fields::subscriptionId] = info.fsm.mapped_id();
        msg[fields::operationId]    = id;
        msg[fields::data]           = std::move(data);

        super::connection().send(msg);

        return std::make_pair(id, std::move(lock));
    }

    virtual std::unique_lock<std::recursive_mutex> sg_cancel_scatter(
        terminal_type& terminal, base::Id operationId)
    {
        auto lock = super::make_lock_guard();

        auto& info = super::get_terminal_info(terminal.id());

        if (!info.ext.scatterOperations.erase(operationId)) {
            throw api::ExceptionT<CHIRP_ERR_INVALID_ID>{};
        }

        CHIRP_ASSERT(m_scatterOperations.count(operationId));
        m_scatterOperations[operationId] = nullptr;

        return lock;
    }

    virtual void sg_respond_to_scattered_message(
        typename TTypes::terminal_type& terminal, base::Id operationId,
        gather_flags flags, bool acquireMutex, base::Buffer&& data)
    {
		using namespace messaging;

        CHIRP_ASSERT(!(flags & GATHER_FINISHED));
        CHIRP_ASSERT(!(flags & GATHER_BINDINGDESTROYED));
        CHIRP_ASSERT(!(flags & GATHER_CONNECTIONLOST));

        auto fn = [&] {
            // find operation in terminal
            auto& tm = super::get_terminal_info(terminal.id());
            auto tmOpIt = tm.ext.gatherOperations.find(operationId);
            if (tmOpIt == tm.ext.gatherOperations.end()) {
                throw api::ExceptionT<CHIRP_ERR_INVALID_ID>{};
            }

            // remove operation from binding
            auto& bd = super::get_binding_info(tmOpIt->second);

            erase_from_multimap(&bd.ext.gatherOperations, operationId,
                terminal.id());

            // remove operation from terminal
            tm.ext.gatherOperations.erase(tmOpIt);

            // remove operation from leaf
            erase_from_multimap(&m_gatherOperations, operationId, &terminal);

            // send Gather message
            typename TTypes::Gather msg;
            msg[fields::operationId] = operationId;
            msg[fields::gatherFlags] = flags;
            msg[fields::data]        = std::move(data);

            if (!m_gatherOperations.count(operationId)) {
                msg[fields::gatherFlags] |= GATHER_FINISHED;
            }

            super::connection().send(msg);
        };

        if (acquireMutex) {
            auto lock = super::make_lock_guard();
            fn();
        }
        else {
            fn();
        }
    }
};

} // namespace scatter_gather
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_SCATTER_GATHER_LEAFLOGIC_HPP
