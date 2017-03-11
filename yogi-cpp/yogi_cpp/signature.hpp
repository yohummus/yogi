#ifndef YOGICPP_SIGNATURE_HPP
#define YOGICPP_SIGNATURE_HPP

#include <string>
#include <ostream>


namespace yogi {

class Signature
{
public:
    typedef unsigned raw_type;

private:
    raw_type m_raw;

public:
    Signature()
    : m_raw(0)
    {
    }

    explicit Signature(raw_type raw)
    : m_raw(raw)
    {
    }

    raw_type raw() const
    {
        return m_raw;
    }

    std::string to_string() const;

    bool operator== (const Signature& other) const
    {
        return m_raw == other.m_raw;
    }

    bool operator!= (const Signature& other) const
    {
        return !(*this == other);
    }
};

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Signature& signature);

#endif // YOGICPP_SIGNATURE_HPP
