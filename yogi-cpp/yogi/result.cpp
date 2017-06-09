#include "result.hpp"

#include <yogi_core.h>

#include <cassert>
#include <sstream>


namespace yogi {

std::string Result::to_string() const
{
    std::stringstream ss;
    ss << '[' << value() << "] " << YOGI_GetErrorString(value());
    return ss.str();
}

const char* Failure::what() const throw()
{
    return YOGI_GetErrorString(this->value());
}

} //namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Result& res)
{
    return os << res.to_string();
}
