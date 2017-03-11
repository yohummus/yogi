#ifndef YOGICPP_INTERNAL_UTILITY_HPP
#define YOGICPP_INTERNAL_UTILITY_HPP

#include "../result.hpp"
#include "../optional.hpp"


namespace yogi {
namespace internal {
namespace {

void throw_on_failure(int res)
{
    if (res < 0) {
        throw Failure(res);
    }
}

} // anonymous namespace

const char* get_raw_string_pointer(const Optional<std::string>& str);
unsigned get_string_size(const Optional<std::string>& str);
std::string tcp_target_to_host(const Optional<std::string>& target);
unsigned tcp_target_to_port(const Optional<std::string>& target);

} // namespace internal
} // namespace yogi

#endif // YOGICPP_INTERNAL_UTILITY_HPP
