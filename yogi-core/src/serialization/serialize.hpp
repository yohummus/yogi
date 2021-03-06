#ifndef YOGI_SERIALIZATION_SERIALIZE_HPP
#define YOGI_SERIALIZATION_SERIALIZE_HPP

#include "../config.h"
#include "serialize_one.hpp"


namespace yogi {
namespace serialization {

template <typename... TValues>
void serialize(std::vector<char>& buffer, const TValues&... values)
{
    // nasty trick to call serialize_one() for every field
    auto _ = { (serialize_one(buffer, values), 0)... };
}

} // namespace serialization
} // namespace yogi

#endif // YOGI_SERIALIZATION_SERIALIZE_HPP
