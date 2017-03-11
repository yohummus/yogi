#ifndef YOGICPP_API_HPP
#define YOGICPP_API_HPP

#include "result.hpp"
#include "types.hpp"

#include <string>


namespace yogi {

const std::string& get_version();
Result set_log_file(const std::string& file, verbosity verb);

} // namespace yogi

#endif // YOGICPP_API_HPP
