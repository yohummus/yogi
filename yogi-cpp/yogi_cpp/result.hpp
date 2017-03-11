#ifndef YOGICPP_RESULT_HPP
#define YOGICPP_RESULT_HPP

#include <exception>
#include <iostream>
#include <cassert>
#include <string>

#include <yogi_core.h>


namespace yogi {

class Result
{
private:
    int m_value;

public:
    explicit Result(int value)
    : m_value(value)
    {
    }

    bool operator== (int value) const
    {
        return m_value == value;
    }

    bool operator!= (int value) const
    {
        return !(*this == value);
    }

    bool operator== (const Result& rhs) const
    {
        return m_value == rhs.m_value;
    }

    bool operator!= (const Result& rhs) const
    {
        return !(*this == rhs);
    }

    explicit operator bool() const
    {
        return m_value >= 0;
    }

    int value() const
    {
        return m_value;
    }

    std::string to_string() const;
};


class Success : public Result
{
public:
    Success()
    : Success(0)
    {
    }

    explicit Success(int value)
    : Result(value)
    {
        assert (value >= 0);
    }
};


class Failure : public Result, public std::exception
{
public:
    explicit Failure(int value)
    : Result(value)
    {
        assert (value < 0);
    }

    virtual const char* what() const throw() override;
};

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::Result& res);

#endif // YOGICPP_RESULT_HPP
