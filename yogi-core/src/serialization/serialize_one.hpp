#ifndef CHIRP_SERIALIZATION_SERIALIZE_ONE_HPP
#define CHIRP_SERIALIZATION_SERIALIZE_ONE_HPP

#include "../config.h"
#include "../base/Id.hpp"
#include "../base/Identifier.hpp"
#include "../base/Buffer.hpp"
#include "../core/scatter_gather/gather_flags.hpp"

#include <vector>


namespace chirp {
namespace serialization {

template <typename T>
void serialize_one(std::vector<char>& buffer, const T& value);

template <>
inline void serialize_one<bool>(
    std::vector<char>& buffer, const bool& value)
{
    buffer.push_back(static_cast<char>(value ? 1 : 0));
}

template <>
inline void serialize_one<std::size_t>(std::vector<char>& buffer,
    const std::size_t& value)
{
    std::size_t length = 1 + (value >= (1<<7)) + (value >= (1<<14))
        + (value >= (1<<21)) + (value >= (1<<28));

    buffer.resize(buffer.size() + length);
    auto it = buffer.end() - length;

    for (std::size_t i = length; i > 0; --i) {
        char byte = (value >> ((i-1) * 7)) & 0x7F;
        byte |= (i > 1 ? (1<<7) : 0);
        *it++ = byte;
    }
}

template <>
inline void serialize_one<base::Id>(std::vector<char>& buffer,
    const base::Id& value)
{
    serialize_one(buffer, value.number());
}

template <>
inline void serialize_one<base::Identifier>(std::vector<char>& buffer,
    const base::Identifier& value)
{
    serialize_one(buffer, value.signature());
    serialize_one(buffer, value.hidden());
    serialize_one(buffer, value.name().size());
    buffer.insert(buffer.end(), value.name().begin(), value.name().end());
}

template <>
inline void serialize_one<base::Buffer>(std::vector<char>& buffer,
    const base::Buffer& value)
{
    serialize_one(buffer, value.size());
    buffer.insert(buffer.end(), value.data(), value.data() + value.size());
}

template <>
inline void serialize_one<core::scatter_gather::gather_flags>(
    std::vector<char>& buffer, const core::scatter_gather::gather_flags& value)
{
    CHIRP_ASSERT(static_cast<int>(value) <= 127);
    buffer.push_back(static_cast<char>(value));
}

} // namespace serialization
} // namespace chirp

#endif // CHIRP_SERIALIZATION_SERIALIZE_ONE_HPP
