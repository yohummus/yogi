#ifndef YOGI_BASE_IDENTIFIER_HPP
#define YOGI_BASE_IDENTIFIER_HPP

#include "../config.h"

#include <string>
#include <functional>
#include <ostream>


namespace yogi {
namespace base {

/***************************************************************************//**
 * Identifier used to identify terminals
 ******************************************************************************/
class Identifier
{
public:
    typedef std::size_t signature_type;
    typedef std::string name_type;

private:
    signature_type m_signature;
    name_type      m_name;
    bool           m_hidden;

public:
    Identifier()
        : m_hidden{false}
    {
    }

    template <typename TSignature, typename TName>
    Identifier(TSignature&& signature, TName&& name, bool hidden)
        : m_signature{std::forward<TSignature>(signature)}
        , m_name     {std::forward<TName>(name)}
        , m_hidden   {hidden}
    {
    }

    const signature_type& signature() const
    {
        return m_signature;
    }

    const name_type& name() const
    {
        return m_name;
    }

    bool hidden() const
    {
        return m_hidden;
    }

    bool operator== (const Identifier& rhs) const
    {
        return (m_signature == rhs.m_signature) && (m_name == rhs.m_name) && (m_hidden == rhs.m_hidden);
    }

    bool operator!= (const Identifier& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace base
} // namespace yogi

namespace std {

template <>
struct hash<yogi::base::Identifier>
{
    size_t operator() (const yogi::base::Identifier& identifier) const
    {
        using namespace yogi::base;
        return hash<Identifier::signature_type>{}(identifier.signature())
            ^ hash<Identifier::name_type>{}(identifier.name());
    }
};

} // namespace std

inline std::ostream& operator<< (std::ostream& os,
    const yogi::base::Identifier& identifier)
{
    os << '(' << identifier.signature() << ", " << identifier.name();
    if (identifier.hidden()) {
        os << ", hidden)";
    }
    else {
        os << ')';
    }

    return os;
}

#endif // YOGI_BASE_IDENTIFIER_HPP
