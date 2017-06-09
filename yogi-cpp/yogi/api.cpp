#include "api.hpp"

#include <yogi_core.h>


namespace yogi {

const std::string& get_version()
{
    static std::string s = YOGI_GetVersion();
    return s;
}

Result set_log_file(const std::string& file, verbosity verb)
{
    YOGI_Initialise();
    int vb = static_cast<int>(verb);
    return Result(YOGI_SetLogFile(file.c_str(), vb));
}

} // namespace yogi
