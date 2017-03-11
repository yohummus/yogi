#ifndef YOGICPP_INTERNAL_GLOB_HPP
#define YOGICPP_INTERNAL_GLOB_HPP

#include <vector>
#include <string>


namespace yogi {
namespace internal {

std::vector<std::string> glob(const std::vector<std::string>& patterns);

} // namespace internal
} // namespace yogi

#endif // YOGICPP_INTERNAL_GLOB_HPP
