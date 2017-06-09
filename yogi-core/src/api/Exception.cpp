#include "Exception.hpp"


namespace yogi {
namespace api {

Exception::descriptions_map& Exception::descriptions()
{
    static descriptions_map map;
    return map;
}

Exception Exception::register_exception(error_code_type errorCode,
    description_type description)
{
    YOGI_ASSERT(!descriptions().count(errorCode));
    descriptions()[errorCode] = description;
    return Exception(errorCode);
}

Exception::description_type Exception::get_description(
    error_code_type errorCode)
{
    auto it = descriptions().find(errorCode);
    if (it == descriptions().end()) {
        return "Unknown error code";
    }
    else {
        return it->second;
    }
}

} // namespace api
} // namespace yogi
