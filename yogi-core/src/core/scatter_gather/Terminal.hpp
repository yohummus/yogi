#ifndef YOGI_CORE_SCATTER_GATHER_TERMINAL_HPP
#define YOGI_CORE_SCATTER_GATHER_TERMINAL_HPP

#include "../../config.h"
#include "../../base/Buffer.hpp"
#include "../../base/AsyncOperation.hpp"
#include "../../base/ObjectRegister.hpp"
#include "../../api/ExceptionT.hpp"
#include "../common/SubscribableTerminalBaseT.hpp"
#include "logic_types.hpp"
#include "gather_flags.hpp"

#include <boost/asio/buffer.hpp>

#include <atomic>
#include <memory>
#include <unordered_map>


namespace yogi {
namespace core {

class Leaf;

namespace scatter_gather {

/***************************************************************************//**
 * Implements a scatter-gather terminal
 ******************************************************************************/
template <typename TTypes=logic_types<>>
class Terminal : public common::SubscribableTerminalBaseT
{
public:
    typedef std::function<bool (const api::Exception&, base::Id,
        gather_flags, std::size_t)> receive_gathered_message_handler_fn;
    typedef std::function<void (const api::Exception&, base::Id,
        std::size_t)> receive_scattered_message_handler_fn;

    struct task_type {
        base::AsyncOperation<receive_gathered_message_handler_fn> operation;
        boost::asio::mutable_buffers_1 gatherBuffer{
            boost::asio::mutable_buffer{}};
    };

private:
    static thread_local bool mst_threadHasLeafLock;

    const base::Id m_id;

    std::recursive_mutex                                       m_tasksMutex;
    std::unordered_map<base::Id, std::unique_ptr<task_type>>   m_tasks;

    std::recursive_mutex                                       m_replyMutex;
    boost::asio::mutable_buffers_1                             m_replyBuffer;
    base::AsyncOperation<receive_scattered_message_handler_fn> m_replyOp;

public:
    template <typename _TBase = SubscribableTerminalBaseT>
    Terminal(Leaf& leaf, base::Identifier identifier)
        : _TBase(leaf, identifier)
        , m_id{register_me<Leaf, LeafLogic<TTypes>>(
            static_cast<typename TTypes::terminal_type&>(*this))}
        , m_replyBuffer{boost::asio::mutable_buffer{}}
    {
    }

    virtual ~Terminal() override
    {
        deregister_me<Leaf, TTypes>(
            static_cast<typename TTypes::terminal_type&>(*this));

        std::lock_guard<std::recursive_mutex> lock{m_tasksMutex};

        for (auto& entry : m_tasks) {
            auto& task = *entry.second;
            task.operation.template fire<YOGI_ERR_CANCELED>(entry.first,
                GATHER_NO_FLAGS, 0);
            task.operation.await_idle();
        }

        m_replyOp.fire<YOGI_ERR_CANCELED>(base::Id{}, 0);
        m_replyOp.await_idle();
    }

    virtual base::Id id() const override
    {
        return m_id;
    }

    base::Id async_scatter_gather(base::Buffer&& scatData,
        boost::asio::mutable_buffers_1 gathBuf,
        receive_gathered_message_handler_fn handlerFn)
    {
        // res.second holds LeafLogic lock guard
        auto res = static_cast<typename TTypes::leaf_logic_type&>(
            static_cast<Leaf&>(leaf())).sg_scatter(
                static_cast<typename TTypes::terminal_type&>(*this),
                std::move(scatData));
        base::Id id = res.first;

        std::lock_guard<std::recursive_mutex> lock{m_tasksMutex};

        YOGI_ASSERT(!m_tasks.count(id));
        auto taskIt = m_tasks.emplace(std::make_pair(id,
            std::make_unique<task_type>())).first;
        auto& task = *taskIt->second;

        try {
            task.gatherBuffer = gathBuf;
            task.operation.arm(handlerFn);
        }
        catch (...) {
            // TODO: close connection
            throw;
        }

        return id;
    }

    void cancel_scatter_gather(base::Id operationId)
    {
        auto lock_ = static_cast<typename TTypes::leaf_logic_type&>(
            static_cast<Leaf&>(leaf())).sg_cancel_scatter(
                static_cast<typename TTypes::terminal_type&>(*this),
                operationId);

        std::lock_guard<std::recursive_mutex> lock{m_tasksMutex};

        auto taskIt = m_tasks.find(operationId);
        YOGI_ASSERT(taskIt != m_tasks.end());
        auto& task = *taskIt->second;

        task.operation.template fire<YOGI_ERR_CANCELED>(operationId,
            GATHER_NO_FLAGS, 0);

        m_tasks.erase(taskIt);
    }

    void async_receive_scattered_message(boost::asio::mutable_buffers_1 buffer,
        receive_scattered_message_handler_fn handlerFn)
    {
        std::lock_guard<std::recursive_mutex> lock{m_replyMutex};

        m_replyOp.arm(handlerFn);
        m_replyBuffer = buffer;
    }

    void cancel_receive_scattered_message()
    {
        std::lock_guard<std::recursive_mutex> lock{m_replyMutex};

        m_replyOp.fire<YOGI_ERR_CANCELED>(base::Id{}, 0);
        m_replyBuffer = boost::asio::mutable_buffers_1{
            boost::asio::mutable_buffer{}};
    }

    void respond_to_scattered_message(base::Id operationId,
        base::Buffer&& data)
    {
        static_cast<typename TTypes::leaf_logic_type&>(
            static_cast<Leaf&>(leaf())).sg_respond_to_scattered_message(
                static_cast<typename TTypes::terminal_type&>(*this),
                operationId, GATHER_NO_FLAGS, !mst_threadHasLeafLock,
                std::move(data));
    }

    void ignore_scattered_message(base::Id operationId)
    {
        static_cast<typename TTypes::leaf_logic_type&>(
            static_cast<Leaf&>(leaf())).sg_respond_to_scattered_message(
                static_cast<typename TTypes::terminal_type&>(*this),
                operationId, GATHER_IGNORED, !mst_threadHasLeafLock,
                base::Buffer{});
    }

    virtual void on_scattered_message_received(base::Id operationId,
        base::Buffer&& data)
    {
        mst_threadHasLeafLock = true;

        std::lock_guard<std::recursive_mutex> lock{m_replyMutex};

        std::size_t n = boost::asio::buffer_copy(m_replyBuffer,
            boost::asio::buffer(data.data(), data.size()));
        m_replyBuffer = boost::asio::mutable_buffers_1{
            boost::asio::mutable_buffer{}};

        if (!m_replyOp.armed()) {
            static_cast<typename TTypes::leaf_logic_type&>(
                static_cast<Leaf&>(leaf())).sg_respond_to_scattered_message(
                    static_cast<typename TTypes::terminal_type&>(*this),
                    operationId, GATHER_DEAF, false, base::Buffer{});
        }

        if (n == data.size()) {
            m_replyOp.fire<YOGI_OK>(operationId, data.size());
        }
        else {
            m_replyOp.fire<YOGI_ERR_BUFFER_TOO_SMALL>(operationId,
                data.size());
        }

        mst_threadHasLeafLock = false;
    }

    virtual bool on_gathered_message_received(base::Id operationId,
        gather_flags flags, base::Buffer&& data)
    {
        mst_threadHasLeafLock = true;

        std::lock_guard<std::recursive_mutex> lock{m_tasksMutex};

        auto taskIt = m_tasks.find(operationId);
        YOGI_ASSERT(taskIt != m_tasks.end());
        auto& task = *taskIt->second;

        std::size_t n = boost::asio::buffer_copy(task.gatherBuffer,
            boost::asio::buffer(data.data(), data.size()));

        bool abort;
        if (n == data.size()) {
            abort = !task.operation.template fire_and_reload<YOGI_OK>(operationId,
                flags, data.size());
        }
        else {
            abort = !task.operation.template fire_and_reload<YOGI_ERR_BUFFER_TOO_SMALL>(
                operationId, flags, data.size());
        }

        if (abort || (flags & GATHER_FINISHED)) {
            task.operation.disarm();
            m_tasks.erase(taskIt);
        }

        mst_threadHasLeafLock = false;

        return !abort;
    }
};

template <typename TTypes>
thread_local bool Terminal<TTypes>::mst_threadHasLeafLock;

} // namespace scatter_gather
} // namespace core
} // namespace yogi

#endif // YOGI_CORE_SCATTER_GATHER_TERMINAL_HPP
