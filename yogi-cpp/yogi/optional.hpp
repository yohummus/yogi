#ifndef YOGI_OPTIONAL_HPP
#define YOGI_OPTIONAL_HPP

#include <utility>
#include <exception>
#include <cassert>


namespace yogi {

class BadOptionalAccess : public std::exception
{
public:
    virtual const char* what() const throw()
    {
        return "Optional<T>::value: not engaged";
    }
};

struct None
{
    constexpr None()
    {
    }
};

constexpr None none;

template <typename T>
class Optional final
{
private:
    bool m_hasValue;
    T    m_value;

public:
    Optional()
    : m_hasValue(false)
    {
    }

    Optional(const T& value)
    : m_hasValue(true)
    , m_value(value)
    {
    }

    Optional(T&& value)
    : m_hasValue(true)
    , m_value(std::forward<T>(value))
    {
    }

    Optional(const Optional& other)
    : m_hasValue(other.m_hasValue)
    , m_value(other.m_value)
    {
    }

    Optional(Optional&& other)
    : m_hasValue(other.m_hasValue)
    , m_value(std::move(other.m_value))
    {
    }

    Optional(None)
    : Optional()
    {
    }

    ~Optional()
    {
    }

    Optional& operator= (const Optional& other)
    {
        m_hasValue = other.m_hasValue;
        m_value = other.m_value;
        return *this;
    }

    Optional& operator= (Optional&& other)
    {
        m_hasValue = other.m_hasValue;
        m_value = std::move(other.m_value);
        return *this;
    }

    Optional& operator= (None)
    {
        *this = Optional();
        return *this;
    }

    const T* operator-> () const
    {
        assert(m_hasValue);
        return &m_value;
    }

    T* operator-> ()
    {
        assert(m_hasValue);
        return &m_value;
    }

    const T& operator* () const
    {
        assert(m_hasValue);
        return m_value;
    }

    T& operator* ()
    {
        assert(m_hasValue);
        return m_value;
    }

    explicit operator bool() const
    {
        return m_hasValue;
    }

    bool has_value() const
    {
        return m_hasValue;
    }

    const T& value() const
    {
        if (!m_hasValue) {
            throw BadOptionalAccess();
        }

        return m_value;
    }

    T& value()
    {
        if (!m_hasValue) {
            throw BadOptionalAccess();
        }

        return m_value;
    }

    template <typename U>
    T value_or(U&& defaultValue)
    {
        if (m_hasValue) {
            return m_value;
        }
        else {
            return defaultValue;
        }
    }

    void reset()
    {
        m_value = T();
        m_hasValue = false;
    }

    bool operator== (const Optional& rhs) const
    {
        return m_hasValue == rhs.m_hasValue && m_value == rhs.m_value;
    }

    bool operator!= (const Optional& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator== (const T& rhs) const
    {
        return m_hasValue && m_value == rhs;
    }

    bool operator!= (const T& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace yogi

#endif //YOGICPP_OPTIONAL_HPP
