#ifndef YOGI_ERRORS_HPP
#define YOGI_ERRORS_HPP

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

#endif // YOGI_ERRORS_HPP
