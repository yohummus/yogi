#include "leaf.hpp"

#include <yogi_core.h>


namespace yogi {

Leaf::Leaf(Scheduler& scheduler)
: Endpoint(YOGI_CreateLeaf, scheduler)
{
}

Leaf::~Leaf()
{
    this->_destroy();
}

const std::string& Leaf::class_name() const
{
    static std::string s = "Leaf";
    return s;
}

} // namespace yogi
