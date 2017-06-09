#ifndef YOGI_LEAF_HPP
#define YOGI_LEAF_HPP

#include "endpoint.hpp"


namespace yogi {

class Scheduler;

class Leaf : public Endpoint
{
public:
    Leaf(Scheduler& scheduler);
    virtual ~Leaf();

    virtual const std::string& class_name() const override;
};

} // namespace yogi

#endif // YOGI_LEAF_HPP
