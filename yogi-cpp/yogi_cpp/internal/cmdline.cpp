#include "cmdline.hpp"
#include "ptree.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>
namespace pt = boost::property_tree;
namespace po = boost::program_options;

#include <iostream>
#include <sstream>
#include <cstdlib>


namespace yogi {
namespace internal {

CommandLineOptions parse_cmdline(int argc, const char* const argv[], bool configFileRequired)
{
    CommandLineOptions opts;

    po::options_description visibleOptions("Allowed options");
    visibleOptions.add_options()(
        "help,h",
         "Show this help message"
        )(
        "connection_target,c", po::value<std::string>()->notifier([&](auto& val) { opts.connectionTarget = val; }),
        "YOGI server to connect to (e.g. 'hostname:12000)"
        )(
        "connection_timeout,t", po::value<float>()->notifier([&](auto& val) { opts.connectionTimeout = val; }),
        "Connection timeout in seconds (-1 for infinity)"
        )(
        "connection_identification,i", po::value<std::string>()->notifier([&](auto& val) { opts.connectionIdentification = val; }),
        "Identification for YOGI connections"
        )(
        "location,l", po::value<std::string>()->notifier([&](auto& val) { opts.location = val; }),
        "Location of the terminals for this process in the YOGI terminal tree"
        )(
        "json,j", po::value<std::vector<std::string>>(&opts.jsonOverrides),
        "Configuration overrides (in JSON format, e.g. '{ \"my-age\": 42 }')"
        );

    po::options_description hiddenOptions("Hidden options");
    hiddenOptions.add_options()(
        "_cfg_files", po::value<std::vector<std::string>>(&opts.configFilePatterns),
        "Configuration files (JSON format)"
        );

    po::positional_options_description positionalOptions;
    positionalOptions.add("_cfg_files", -1);

    po::variables_map vm;

    po::options_description options;
    options.add(visibleOptions);
    options.add(hiddenOptions);

    po::store(po::command_line_parser(argc, argv).options(options).positional(positionalOptions).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::string binaryName = argv[0];
        auto pos = binaryName.find_last_of("/\\");
        if (pos != std::string::npos) {
            binaryName = binaryName.substr(pos + 1);
        }

        std::string firstConfigFile;
        if (configFileRequired) {
            firstConfigFile = "config.json";
        }
        else {
            firstConfigFile = "[config.json]";
        }

        std::cout << "Usage: " << binaryName << " [options] " << firstConfigFile << " [config2.json ...]" << std::endl;
        std::cout << std::endl;
        std::cout << "Command line arguments override corresponding settings in the configuration files." << std::endl;
        std::cout << std::endl;
        std::cout << visibleOptions << std::endl;
        std::exit(0);
    }

    if (configFileRequired && opts.configFilePatterns.empty()) {
        throw std::runtime_error("No configuration files specified");
    }

    return opts;
}

void update_ptree(pt::ptree* config, const CommandLineOptions& opts)
{
    for (auto& jsonStr : opts.jsonOverrides) {
        std::stringstream ss(jsonStr);
        pt::ptree partialTree;
        pt::read_json(ss, partialTree);
        merge_property_trees(config, partialTree);
    }

    if (opts.connectionTarget) {
        config->put("yogi.connection.target", *opts.connectionTarget);
    }

    if (opts.connectionIdentification) {
        config->put("yogi.connection.identification", *opts.connectionIdentification);
    }

    if (opts.connectionTimeout) {
        config->put("yogi.connection.timeout", *opts.connectionTimeout);
    }

    if (opts.location) {
        config->put("yogi.location", *opts.location);
    }
}

} // namespace internal
} // namespace yogi
