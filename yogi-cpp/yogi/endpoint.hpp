#ifndef YOGI_ENDPOINT_HPP
#define YOGI_ENDPOINT_HPP

#include "scheduler.hpp"


namespace yogi {

class Scheduler;

class Endpoint : public Object
{
private:
    Scheduler& m_scheduler;

protected:
    template <typename ApiFn>
    Endpoint(ApiFn apiFn, Scheduler& scheduler)
    : Object(apiFn, scheduler.handle())
    , m_scheduler(scheduler)
    {
    }

public:
    Scheduler& scheduler()
    {
        return m_scheduler;
    }
};

} // namespace yogi

#endif // YOGI_ENDPOINT_HPP
