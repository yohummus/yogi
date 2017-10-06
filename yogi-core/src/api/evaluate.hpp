#ifndef YOGI_API_EVALUATE_HPP
#define YOGI_API_EVALUATE_HPP

#include "../config.h"
#include "../yogi_core.h"
#include "Exception.hpp"


namespace yogi {
namespace api {
namespace internal {

template <class TR, class TF>
struct evaluator;

template <class TF>
struct evaluator<void, TF>
{
    int operator() (TF fn) const
    {
        fn();
        return YOGI_OK;
    }
};

template <class TF>
struct evaluator<int, TF>
{
    int operator() (TF fn) const
    {
        return fn();
    }
};

} // namespace internal

template <class TF>
int evaluate(TF fn)
{
    try {
        return internal::evaluator<decltype(fn()), TF>()(fn);
    }
    catch (const Exception& e) {
        return e.error_code();
    }
    catch (const std::bad_alloc&) {
        return YOGI_ERR_BAD_ALLOCATION;
    }
    catch (...) {
        return YOGI_ERR_UNKNOWN;
    }
}

} // namespace api
} // namespace yogi

#endif // YOGI_API_EVALUATE_HPP
