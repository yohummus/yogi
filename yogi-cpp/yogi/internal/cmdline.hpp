#ifndef YOGI_INTERNAL_CMDLINE_HPP
#define YOGI_INTERNAL_CMDLINE_HPP

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <string>


namespace yogi {
namespace internal {

struct CommandLineOptions {
    boost::optional<std::string> connectionTarget;
    boost::optional<std::string> connectionIdentification;
    boost::optional<float>       connectionTimeout;
    boost::optional<std::string> location;
    std::vector<std::string>     configFilePatterns;
    std::vector<std::string>     jsonOverrides;
};

CommandLineOptions parse_cmdline(int argc, const char* const argv[], bool configFileRequired);
void update_ptree(boost::property_tree::ptree* config, const CommandLineOptions& opts);

} // namespace internal
} // namespace yogi

#endif // YOGI_INTERNAL_CMDLINE_HPP
