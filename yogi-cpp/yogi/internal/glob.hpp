#ifndef YOGI_INTERNAL_GLOB_HPP
#define YOGI_INTERNAL_GLOB_HPP

#include <vector>
#include <string>


namespace yogi {
namespace internal {

std::vector<std::string> glob(const std::vector<std::string>& patterns);

} // namespace internal
} // namespace yogi

#endif // YOGI_INTERNAL_GLOB_HPP
