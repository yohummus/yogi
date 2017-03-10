#ifndef CHIRP_API_EVALUATE_HPP
#define CHIRP_API_EVALUATE_HPP

#include "../config.h"
#include "../chirp.h"
#include "Exception.hpp"


namespace chirp {
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
        return CHIRP_OK;
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
        return CHIRP_ERR_BAD_ALLOCATION;
    }
    catch (...) {
        return CHIRP_ERR_UNKNOWN;
    }
}

} // namespace api
} // namespace chirp

#endif // CHIRP_API_EVALUATE_HPP
