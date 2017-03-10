#ifndef CHIRP_BASE_ASYNCHRONOUSOPERATION_HPP
#define CHIRP_BASE_ASYNCHRONOUSOPERATION_HPP

#include "../config.h"
#include "../api/ExceptionT.hpp"

#include <boost/asio/io_service.hpp>

#include <mutex>
#include <condition_variable>


namespace chirp {
namespace base {
namespace internal {

template <typename THandlerFn>
class AsyncOperationBase
{
    typedef THandlerFn                  handler_fn;

protected:
    handler_fn              m_armedHandler;
    int                     m_runningOperations;
    mutable std::mutex      m_mutex;
    std::condition_variable m_cv;

protected:
    handler_fn take_armed_handler()
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        handler_fn tmp;
        std::swap(tmp, m_armedHandler);
        return tmp;
    }

    handler_fn get_armed_handler()
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_armedHandler;
    }

public:
    AsyncOperationBase()
        : m_runningOperations{0}
    {
    }

    ~AsyncOperationBase()
    {
        CHIRP_ASSERT(!m_runningOperations);
    }

    bool armed() const
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        return !!m_armedHandler;
    }

    template <typename THandlerFn_>
    void arm(THandlerFn_&& handlerFn)
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        if (m_armedHandler) {
            throw api::ExceptionT<CHIRP_ERR_ASYNC_OPERATION_RUNNING>{};
        }

        m_armedHandler = std::forward<THandlerFn_>(handlerFn);
        ++m_runningOperations;
    }

    void disarm()
    {
        {{
            std::lock_guard<std::mutex> lock{m_mutex};

            if (m_armedHandler) {
                m_armedHandler = handler_fn{};
                --m_runningOperations;
            }
        }}

        m_cv.notify_all();
    }

    void await_idle()
    {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_cv.wait(lock, [&] { return !m_runningOperations; });
    }
};

template <typename TReturnType, typename THandlerFn>
class AsyncOperationHelper : public AsyncOperationBase<THandlerFn>
{
    typedef AsyncOperationBase<THandlerFn> super;

public:
    template <int TErrorCode, typename... THandlerArgs>
    TReturnType fire(THandlerArgs&&... handlerArgs)
    {
        auto handler = super::take_armed_handler();
        if (handler) {
            TReturnType result = handler(api::ExceptionT<TErrorCode>{},
                std::forward<THandlerArgs>(handlerArgs)...);

            {{
                std::lock_guard<std::mutex> lock{super::m_mutex};
                --super::m_runningOperations;
            }}

            super::m_cv.notify_all();

            return result;
        }

        return TReturnType{};
    }

    template <int TErrorCode, typename... THandlerArgs>
    TReturnType fire_and_reload(THandlerArgs&&... handlerArgs)
    {
        auto handler = super::get_armed_handler();
        if (handler) {
            return handler(api::ExceptionT<TErrorCode>{},
                std::forward<THandlerArgs>(handlerArgs)...);
        }

        return TReturnType{};
    }
};

template <typename THandlerFn>
class AsyncOperationHelper<void, THandlerFn>
    : public AsyncOperationBase<THandlerFn>
{
    typedef AsyncOperationBase<THandlerFn> super;

public:
    template <int TErrorCode, typename... THandlerArgs>
    void fire(THandlerArgs&&... handlerArgs)
    {
        auto handler = super::take_armed_handler();
        if (handler) {
            handler(api::ExceptionT<TErrorCode>{},
                std::forward<THandlerArgs>(handlerArgs)...);

            {{
                std::lock_guard<std::mutex> lock{super::m_mutex};
                --super::m_runningOperations;
            }}

            super::m_cv.notify_all();
        }
    }

    template <typename... THandlerArgs>
    void fire(const api::Exception& e, THandlerArgs&&... handlerArgs)
    {
        auto handler = super::take_armed_handler();
        if (handler) {
            handler(e, std::forward<THandlerArgs>(handlerArgs)...);

            {{
                std::lock_guard<std::mutex> lock{super::m_mutex};
                --super::m_runningOperations;
            }}

            super::m_cv.notify_all();
        }
    }

    template <int TErrorCode, typename... THandlerArgs>
    void fire_and_reload(THandlerArgs&&... handlerArgs)
    {
        auto handler = super::get_armed_handler();
        if (handler) {
            handler(api::ExceptionT<TErrorCode>{},
                std::forward<THandlerArgs>(handlerArgs)...);
        }
    }
};

} // namespace internal

/***************************************************************************//**
 * Class for managing asynchronous operations
 *
 * @tparam THandlerFn Type of the handler function that will be called
 ******************************************************************************/
template <typename THandlerFn>
class AsyncOperation : public internal::AsyncOperationHelper<
    typename THandlerFn::result_type, THandlerFn>
{
};

} // namespace base
} // namespace chirp

#endif // CHIRP_BASE_ASYNCHRONOUSOPERATION_HPP
