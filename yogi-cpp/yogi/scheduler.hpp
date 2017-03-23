#ifndef YOGI_SCHEDULER_HPP
#define YOGI_SCHEDULER_HPP

#include "object.hpp"


namespace yogi {

class Scheduler : public Object
{
public:
    Scheduler();
    virtual ~Scheduler();

    void set_thread_pool_size(std::size_t n);

    virtual const std::string& class_name() const override;
};

} // namespace yogi

#endif // YOGI_SCHEDULER_HPP
