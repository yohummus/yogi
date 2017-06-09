#ifndef YOGI_BASE_ID_HPP
#define YOGI_BASE_ID_HPP

#include "../config.h"

#include <cstddef>
#include <cassert>
#include <ostream>


namespace yogi {
namespace base {

/***************************************************************************//**
 * ID for identifying things
 *
 * An integral type is used to store the actual value. Those numbers are always
 * greater than zero; a value of zero represents an invalid ID.
 ******************************************************************************/
class Id
{
public:
    typedef std::size_t number_type;

private:
    number_type m_number;

public:
    static number_type invalid_number()
    {
        return 0;
    }

    Id()
        : m_number{invalid_number()}
    {
    }

    explicit Id(number_type number)
        : m_number{number}
    {
        YOGI_ASSERT(number != invalid_number());
    }

    Id(const Id& rhs)
    {
        *this = rhs;
    }

    Id(Id&& rhs)
    {
        *this = std::move(rhs);
    }

    number_type number() const
    {
        return m_number;
    }

    bool valid() const
    {
        return m_number != invalid_number();
    }

    Id& operator= (const Id& rhs)
    {
        m_number = rhs.m_number;
        return *this;
    }

    Id& operator= (Id&& rhs)
    {
        m_number = rhs.m_number;
        rhs.m_number = invalid_number();
        return *this;
    }

    explicit operator bool() const
    {
        return valid();
    }

    bool operator== (const Id& rhs) const
    {
        return m_number == rhs.m_number;
    }

    bool operator!= (const Id& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace base
} // namespace yogi

namespace std {

template <>
struct hash<yogi::base::Id>
{
    std::size_t operator()(const yogi::base::Id& id) const
    {
        return id.number();
    }
};

inline std::ostream& operator<< (std::ostream& os, const yogi::base::Id& id)
{
    if (id.valid()) {
        os << id.number();
    }
    else {
        os << "??";
    }

    return os;
}

} // namespace std

#endif // YOGI_BASE_ID_HPP
