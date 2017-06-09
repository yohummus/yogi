#include "utility.hpp"


namespace yogi {
namespace internal {

const char* get_raw_string_pointer(const Optional<std::string>& str)
{
    if (str) {
        return str->c_str();
    }

    return nullptr;
}

unsigned get_string_size(const Optional<std::string>& str)
{
    if (str) {
        return static_cast<unsigned>(str->size());
    }

    return 0;
}

std::string tcp_target_to_host(const Optional<std::string>& target)
{
    std::string host;

    if (target) {
        host = target->substr(0, target->find_last_of(':'));
    }

    return host;
}

unsigned tcp_target_to_port(const Optional<std::string>& target)
{
    unsigned port = 0;

    if (target) {
        auto pos = target->find_last_of(':');
        if (pos != std::string::npos) {
            auto portStr = target->substr(pos + 1);
            port = static_cast<unsigned>(std::stoi(portStr));
        }
    }

    return port;
}

} // namespace internal
} // namespace yogi
