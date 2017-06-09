#ifndef YOGI_SERIALIZATION_CAN_DESERIALIZE_ONE_HPP
#define YOGI_SERIALIZATION_CAN_DESERIALIZE_ONE_HPP

#include "../config.h"
#include "../base/Id.hpp"
#include "../base/Identifier.hpp"
#include "../base/Buffer.hpp"
#include "../core/scatter_gather/gather_flags.hpp"

#include <vector>


namespace yogi {
namespace serialization {

template <typename T>
bool can_deserialize_one(const std::vector<char>& buffer,
    std::vector<char>::const_iterator start);

template <>
inline bool can_deserialize_one<std::size_t>(const std::vector<char>& buffer,
    std::vector<char>::const_iterator start)
{
    for (auto it = start; it != buffer.end(); ++it) {
        unsigned char byte = static_cast<unsigned char>(*it);
        if (!(byte & (1<<7))) {
            return true;
        }
    }

    return false;
}

} // namespace serialization
} // namespace yogi

#endif // YOGI_SERIALIZATION_CAN_DESERIALIZE_ONE_HPP
