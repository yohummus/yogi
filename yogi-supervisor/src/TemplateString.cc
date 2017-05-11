#include "TemplateString.hh"

#include <boost/algorithm/string/replace.hpp>

using namespace std::string_literals;


TemplateString::TemplateString()
{
}

TemplateString::TemplateString(std::string value)
: m_value(value)
{
}

TemplateString::TemplateString(std::string name, std::string value)
: m_name(name)
, m_value(value)
{
}

std::string TemplateString::name() const
{
    return m_name;
}

std::string TemplateString::value() const
{
    return m_value;
}

void TemplateString::resolve(const TemplateString& other)
{
    boost::replace_all(m_value, "%"s + other.name() + "%", other.value());
}

void TemplateString::resolve(const std::vector<TemplateString>& others)
{
    for (auto& other : others) {
        resolve(other);
    }
}

std::ostream& operator<< (std::ostream& os, const TemplateString& ts)
{
    if (ts.name().empty()) {
        return os << ts.value();
    }
    else {
        return os << ts.name() << " = " << ts.value();
    }
}
