#ifndef YOGI_BASE_BUFFER_HPP
#define YOGI_BASE_BUFFER_HPP

#include "../config.h"

#include <vector>
#include <iomanip>
#include <ostream>


namespace yogi {
namespace base {

/***************************************************************************//**
 * Buffer for binary data
 ******************************************************************************/
class Buffer
{
private:
    std::vector<char> m_data;

public:
    Buffer()
    {
    }

    template <typename TIterator>
    Buffer(TIterator first, TIterator last)
        : m_data(first, last)
    {
    }

    Buffer(const void* data, std::size_t size)
        : m_data(static_cast<const char*>(data),
            static_cast<const char*>(data) + size)
    {
    }

    Buffer(const Buffer& other)
        : m_data(other.m_data)
    {
    }

    Buffer(Buffer&& other)
        : m_data(std::move(other.m_data))
    {
    }

    std::size_t size() const
    {
        return m_data.size();
    }

    const char* data() const
    {
        return m_data.data();
    }

    char operator[] (std::size_t pos) const
    {
        return m_data[pos];
    }

	Buffer& operator= (const Buffer& rhs)
	{
		m_data = rhs.m_data;
		return *this;
	}

	Buffer& operator= (Buffer&& rhs)
	{
		m_data = std::move(rhs.m_data);
		return *this;
	}

    bool operator== (const Buffer& rhs) const
    {
        return m_data == rhs.m_data;
    }

    bool operator!= (const Buffer& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace base
} // namespace yogi

inline std::ostream& operator<< (std::ostream& os,
    const yogi::base::Buffer& buf)
{
    os << "[" << std::hex << std::setw(2) << std::setfill('0');
    for (std::size_t i = 0; i < buf.size(); ++i) {
        os << static_cast<unsigned>(buf[i]);
        if (i != buf.size() - 1) {
            os << ' ';
        }
    }
    os << "]";

    return os;
}

#endif // YOGI_BASE_BUFFER_HPP
