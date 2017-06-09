#include "signature.hpp"

#include <iomanip>
#include <sstream>


namespace yogi {

std::string Signature::to_string() const
{
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << m_raw;
    return ss.str();
}

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Signature& signature)
{
    return os << signature.to_string();
}

