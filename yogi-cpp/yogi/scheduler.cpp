#include "scheduler.hpp"
#include "internal/utility.hpp"


namespace yogi {

Scheduler::Scheduler()
: Object(YOGI_CreateScheduler)
{
}

Scheduler::~Scheduler()
{
    this->_destroy();
}

void Scheduler::set_thread_pool_size(std::size_t n)
{
    int res = YOGI_SetSchedulerThreadPoolSize(this->handle(), static_cast<unsigned>(n));
    internal::throw_on_failure(res);
}

const std::string& Scheduler::class_name() const
{
    static std::string s = "Scheduler";
    return s;
}

} // namespace yogi
