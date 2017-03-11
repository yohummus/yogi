#ifndef YOGICPP_ERRORS_HPP
#define YOGICPP_ERRORS_HPP

#include "result.hpp"


namespace yogi {
namespace errors {

class Canceled : public Failure
{
public:
    Canceled()
    : Failure(YOGI_ERR_CANCELED)
    {
    }
};

class Timeout : public Failure
{
public:
    Timeout()
    : Failure(YOGI_ERR_TIMEOUT)
    {
    }
};

} // namespace errors
} // namespace yogi

#endif // YOGICPP_ERRORS_HPP
