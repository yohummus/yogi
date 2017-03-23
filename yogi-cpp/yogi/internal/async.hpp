#ifndef YOGI_INTERNAL_ASYNC_HPP
#define YOGI_INTERNAL_ASYNC_HPP

#include "../result.hpp"
#include "../logging.hpp"
#include "utility.hpp"

#include <functional>
#include <memory>
#include <type_traits>


namespace yogi {
namespace internal {

template <typename Ret, typename CompletionHandler, typename... ApiArgs>
struct async_call_completion_handler
{
    static Ret fn(int res, ApiArgs... args, void* userArg)
    {
        auto fn_ = std::unique_ptr<CompletionHandler>(static_cast<CompletionHandler*>(userArg));

        try {
            if (res >= 0) {
                return (*fn_)(Success(res), args...);
            }
            else {
                return (*fn_)(Failure(res), args...);
            }
        }
        catch (const std::exception& e) {
            YOGI_LOG(ERROR, Logger::yogi_logger(), "Uncaught exception in callback function: " << e.what());
        }
        catch (...) {
            YOGI_LOG(ERROR, Logger::yogi_logger(), "Caught object not derived from std::exception in callback function");
        }

        return YOGI_ERR_UNKNOWN;
    }
};

template <typename CompletionHandler, typename... ApiArgs>
struct async_call_completion_handler<void, CompletionHandler, ApiArgs...>
{
    static void fn(int res, ApiArgs... args, void* userArg)
    {
        auto fn_ = std::unique_ptr<CompletionHandler>(static_cast<CompletionHandler*>(userArg));

        try {
            if (res >= 0) {
                (*fn_)(Success(res), args...);
            }
            else {
                (*fn_)(Failure(res), args...);
            }
        }
        catch (const std::exception& e) {
            YOGI_LOG(ERROR, Logger::yogi_logger(), "Uncaught exception in callback function: " << e.what());
        }
        catch (...) {
            YOGI_LOG(ERROR, Logger::yogi_logger(), "Caught object not derived from std::exception in callback function");
        }
    }
};

template <typename... ApiArgs, typename CompletionHandler, typename StartFn>
Success async_call(CompletionHandler completionHandler, StartFn startFn)
{
    auto completionHandlerPtr = std::make_unique<CompletionHandler>(completionHandler);
    void* userArg = completionHandlerPtr.get();

    typedef typename std::result_of<CompletionHandler(const Result&, ApiArgs...)>::type ret_type;
    auto fn = async_call_completion_handler<ret_type, CompletionHandler, ApiArgs...>::fn;

    int res = startFn(fn, userArg);
    throw_on_failure(res);

    completionHandlerPtr.release();

    return Success(res);
}

} // namespace internal
} // namespace yogi

#endif // YOGI_INTERNAL_ASYNC_HPP
