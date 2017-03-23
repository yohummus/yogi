#include "path.hpp"

using namespace std::string_literals;


namespace yogi {
namespace internal {

void throw_if_path_invalid(const std::string& str)
{
    if (str.find("//") != std::string::npos) {
        throw BadPath(str);
    }
}

} // namespace

BadPath::BadPath(const std::string& path)
: std::runtime_error("Invalid path: '"s + path + "'")
{
}

Path::Path()
{
}

Path::Path(std::string str)
: m_str(str)
{
    internal::throw_if_path_invalid(m_str);

    if (m_str.size() > 1 && m_str.back() == '/') {
        m_str.pop_back();
    }
}

const std::string& Path::to_string() const
{
    return m_str;
}

bool Path::empty() const
{
    return m_str.empty();
}

std::size_t Path::size() const
{
    return m_str.size();
}

void Path::clear()
{
    m_str.clear();
}

bool Path::is_absolute() const
{
    return !empty() && m_str.front() == '/';
}

bool Path::is_root() const
{
    return m_str == "/";
}

bool Path::operator== (const Path& rhs) const
{
    return m_str == rhs.m_str;
}

bool Path::operator!= (const Path& rhs) const
{
    return !(*this == rhs);
}

Path Path::operator/ (const Path& other) const
{
    if (other.is_absolute()) {
        throw BadPath(other.m_str);
    }

    if (is_root()) {
        return Path(m_str + other.m_str);
    }
    else {
        return Path(m_str + '/' + other.m_str);
    }
}

Path Path::operator/ (const char* other) const
{
    return *this / Path(other);
}

Path& Path::operator/= (const Path& other)
{
    *this = *this / other;
    return *this;
}

Path& Path::operator/= (const char* other)
{
    return *this /= Path(other);
}

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Path& path)
{
    return os << path.to_string();
}
