#ifndef CHIRP_SERIALIZATION_DESERIALIZE_ONE_HPP
#define CHIRP_SERIALIZATION_DESERIALIZE_ONE_HPP

#include "../config.h"
#include "../base/Id.hpp"
#include "../base/Identifier.hpp"
#include "../base/Buffer.hpp"
#include "../core/scatter_gather/gather_flags.hpp"

#include <vector>


namespace chirp {
namespace serialization {

template <typename T>
void deserialize_one(const std::vector<char>& buffer,
    std::vector<char>::const_iterator& it, T& value);

template <>
inline void deserialize_one<bool>(const std::vector<char>& buffer,
    std::vector<char>::const_iterator& it, bool& value)
{
    char byte = *it++;
    value = !!byte;
}

template <>
inline void deserialize_one<std::size_t>(const std::vector<char>& buffer,
    std::vector<char>::const_iterator& it, std::size_t& value)
{
    std::size_t tmp = 0;

    while (it != buffer.end()) {
        unsigned char byte = static_cast<unsigned char>(*it++);

		tmp |= (std::size_t)(byte & ~(1<<7));
        if (!(byte & (1<<7))) {
            value = tmp;
            return;
        }

		tmp <<= 7;
    }
}

template <>
inline void deserialize_one<base::Id>(const std::vector<char>& buffer,
    std::vector<char>::const_iterator& it, base::Id& value)
{
    base::Id::number_type number;
    deserialize_one(buffer, it, number);
    value = base::Id{number};
}

template <>
inline void deserialize_one<base::Identifier>(const std::vector<char>& buffer,
    std::vector<char>::const_iterator& it, base::Identifier& value)
{
    base::Identifier::signature_type signature;
    deserialize_one(buffer, it, signature);

    bool hidden;
    deserialize_one(buffer, it, hidden);
    
    std::size_t nameSize;
    deserialize_one(buffer, it, nameSize);

    base::Identifier::name_type name;
    CHIRP_ASSERT(std::distance(it, buffer.end()) >= 0);
    CHIRP_ASSERT(static_cast<std::size_t>(std::distance(it, buffer.end()))
        >= nameSize);
    name.assign(it, it + nameSize);
    it += nameSize;

    value = base::Identifier{signature, name, hidden};
}

template <>
inline void deserialize_one<base::Buffer>(const std::vector<char>& buffer,
    std::vector<char>::const_iterator& it, base::Buffer& value)
{
    std::size_t size;
    deserialize_one(buffer, it, size);

    CHIRP_ASSERT(std::distance(it, buffer.end()) >= 0);
    CHIRP_ASSERT(static_cast<std::size_t>(std::distance(it, buffer.end()))
        >= size);
    value = base::Buffer{it, it + size};
    it += size;
}

template <>
inline void deserialize_one<core::scatter_gather::gather_flags>(
    const std::vector<char>& buffer, std::vector<char>::const_iterator& it,
    core::scatter_gather::gather_flags& value)
{
    char byte = *it++;
    value = static_cast<core::scatter_gather::gather_flags>(byte);
}

} // namespace serialization
} // namespace chirp

#endif // CHIRP_SERIALIZATION_DESERIALIZE_ONE_HPP
