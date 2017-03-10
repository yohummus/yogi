#ifndef CHIRP_CORE_COMMON_PUBLISHSUBSCRIBETERMINALBASET_HPP
#define CHIRP_CORE_COMMON_PUBLISHSUBSCRIBETERMINALBASET_HPP

#include "../../config.h"
#include "../../base/Buffer.hpp"
#include "../../base/AsyncOperation.hpp"
#include "../../api/ExceptionT.hpp"
#include "SubscribableTerminalBaseT.hpp"

#include <boost/asio/buffer.hpp>

#include <atomic>


namespace chirp {
namespace core {

class Leaf;

namespace common {
namespace internal {

template <bool THasCache>
struct PublishSubscribeTerminalBaseHelper;

template <>
struct PublishSubscribeTerminalBaseHelper<true>
{
    typedef std::function<void (const api::Exception&, std::size_t, bool)>
        rcv_handler_fn;

    template <int TErrorCode>
    static void fire_async_operation(base::AsyncOperation<rcv_handler_fn>* op,
        std::size_t size, bool cached)
    {
        op->fire<TErrorCode>(size, cached);
    }
};

template <>
struct PublishSubscribeTerminalBaseHelper<false>
{
    typedef std::function<void (const api::Exception&, std::size_t)>
        rcv_handler_fn;

    template <int TErrorCode>
    static void fire_async_operation(base::AsyncOperation<rcv_handler_fn>* op,
        std::size_t size, bool)
    {
        op->fire<TErrorCode>(size);
    }
};

} // namespace internal

/***************************************************************************//**
 * Templated base class for terminals based on the publish-subscribe messaging
 * pattern
 ******************************************************************************/
template <typename TTypes, bool THasCache>
class PublishSubscribeTerminalBaseT : public SubscribableTerminalBaseT
{
public:
    typedef TTypes logic_types;
	typedef typename TTypes::leaf_logic_type leaf_logic_type;
	typedef typename TTypes::terminal_type terminal_type;
    typedef internal::PublishSubscribeTerminalBaseHelper<THasCache> helper;
    typedef typename helper::rcv_handler_fn rcv_handler_fn;

private:
    const base::Id m_id;

    std::recursive_mutex                 m_rcvMutex;
    boost::asio::mutable_buffers_1       m_rcvBuffer;
    base::AsyncOperation<rcv_handler_fn> m_rcvPublishedMsgOp;

protected:
    std::unique_lock<std::recursive_mutex> make_lock_guard()
    {
        return std::unique_lock<std::recursive_mutex>{m_rcvMutex};
    }

    template <typename _TBase = SubscribableTerminalBaseT>
    PublishSubscribeTerminalBaseT(Leaf& leaf, base::Identifier identifier)
        : _TBase(leaf, identifier)
        , m_id{register_me<Leaf, leaf_logic_type>(
			static_cast<terminal_type&>(*this))}
        , m_rcvBuffer(boost::asio::mutable_buffer{})
    {
    }

public:
    virtual ~PublishSubscribeTerminalBaseT() override
    {
        helper::template fire_async_operation<CHIRP_ERR_CANCELED>(
            &m_rcvPublishedMsgOp, 0, false);
        m_rcvPublishedMsgOp.await_idle();

        deregister_me<Leaf, TTypes>(static_cast<terminal_type&>(*this));
    }

    virtual base::Id id() const override
    {
        return m_id;
    }

    bool publish(base::Buffer&& data)
    {
        auto& leafLogic = static_cast<leaf_logic_type&>(
			static_cast<Leaf&>(leaf()));
        return leafLogic.publish(static_cast<terminal_type&>(*this),
            std::move(data));
    }

    void async_receive_published_message(
        boost::asio::mutable_buffers_1 buffer, rcv_handler_fn handlerFn)
    {
        auto lock = make_lock_guard();

        m_rcvPublishedMsgOp.arm(handlerFn);
        m_rcvBuffer = buffer;
    }

    void cancel_receive_published_message()
    {
        auto lock = make_lock_guard();

        helper::template fire_async_operation<CHIRP_ERR_CANCELED>(
            &m_rcvPublishedMsgOp, 0, false);

        m_rcvBuffer = boost::asio::mutable_buffers_1{boost::asio::mutable_buffer{}};
    }

    void on_data_received(base::Buffer&& data, bool cached)
    {
        auto lock = make_lock_guard();

        std::size_t n = boost::asio::buffer_copy(m_rcvBuffer,
            boost::asio::buffer(data.data(), data.size()));
        m_rcvBuffer = boost::asio::mutable_buffers_1{boost::asio::mutable_buffer{}};

        if (n == data.size()) {
            helper::template fire_async_operation<CHIRP_OK>(
                &m_rcvPublishedMsgOp, data.size(), cached);
        }
        else {
            helper::template fire_async_operation<CHIRP_ERR_BUFFER_TOO_SMALL>(
                &m_rcvPublishedMsgOp, data.size(), cached);
        }
    }
};

} // namespace common
} // namespace core
} // namespace chirp

#endif // CHIRP_CORE_COMMON_PUBLISHSUBSCRIBETERMINALBASET_HPP
