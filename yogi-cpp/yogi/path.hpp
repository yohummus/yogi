#ifndef YOGI_PATH_HPP
#define YOGI_PATH_HPP

#include <string>
#include <stdexcept>


namespace yogi {

class BadPath : public std::runtime_error
{
public:
    BadPath(const std::string& path);
};

class Path
{
private:
    std::string m_str;

public:
    Path();
    Path(std::string str);

    const std::string& to_string() const;
    bool empty() const;
    std::size_t size() const;
    void clear();

    bool is_absolute() const;
    bool is_root() const;

    bool operator== (const Path& rhs) const;
    bool operator!= (const Path& rhs) const;

    Path operator/ (const Path& other) const;
    Path operator/ (const char* other) const;
    Path& operator/= (const Path& other);
    Path& operator/= (const char* other);
};

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Path& path);

#endif //YOGICPP_PATH_HPP
