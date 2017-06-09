#ifndef YOGI_API_EXCEPTION_HPP
#define YOGI_API_EXCEPTION_HPP

#include "../config.h"

#include <exception>
#include <unordered_map>


namespace yogi {
namespace api {

/***************************************************************************//**
 * Base class for exceptions
 ******************************************************************************/
class Exception : public std::exception
{
public:
    typedef int         error_code_type;
    typedef const char* description_type;

private:
    typedef std::unordered_map<error_code_type, description_type>
        descriptions_map;

private:
    const error_code_type m_errorCode;

private:
    static descriptions_map& descriptions();

protected:
    static Exception register_exception(error_code_type,
        description_type description);

    Exception(error_code_type errorCode)
        : m_errorCode{errorCode}
    {
    }

public:
    static description_type get_description(error_code_type errorCode);

    error_code_type error_code() const
    {
        return m_errorCode;
    }
};

} // namespace api
} // namespace yogi

#endif // YOGI_API_EXCEPTION_HPP
