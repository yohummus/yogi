#include "Pinger.hh"

#include <yogi.hpp>

#include <boost/program_options.hpp>

#include <limits>
#include <regex>

using namespace std::string_literals;


int main(int argc, const char* argv[])
{
    yogi::Logger::set_thread_name("main");

    try {
        float                    interval = 1.0;
        std::size_t              count = std::numeric_limits<std::size_t>::max();
        bool                     useServiceTerminals = false;
        std::size_t              payloadSize = 4;
        std::vector<std::string> targets;

        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        desc.add_options()(
            "help,h",
            "show this help message"
            )(
            "interval,i", po::value<float>(&interval),
            "interval between pings in seconds (default is 1.0)"
            )(
            "count,c", po::value<std::size_t>(&count),
            "number of pings (default is infinity)"
            )(
            "payload,p", po::value<std::size_t>(&payloadSize),
            "number of payload bytes (default is 4)"
            )(
            "service,s", po::bool_switch(&useServiceTerminals)->default_value(false),
            "use Service Terminals instead of Master Terminals"
            );

        po::options_description hidden("Hidden options");
        hidden.add_options()(
            "target", po::value<std::vector<std::string>>(&targets)
            );

        po::positional_options_description p;
        p.add("target", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
            .options(desc.add(hidden))
            .positional(p)
            .run(),
            vm);
        po::notify(vm);

        std::string host;
        unsigned    port;
        std::string echoerLocation;

        bool targetValid = false;
        if (targets.size() == 1) {
            std::smatch m;
            std::regex re("^((.*?):)?(([0-9]*?):)?(.+)$");
            targetValid = std::regex_match(targets[0], m, re);
            if (targetValid) {
                host = m[2].length() ? m[2] : "127.0.0.1"s;
                port = m[4].length() ? static_cast<unsigned>(std::stoul(m[4])) : 10000;
                echoerLocation = m[5];
            }
        }

        if (echoerLocation[0] != '/') {
            echoerLocation = "/Echoers/"s + echoerLocation;
        }

        if (vm.count("help") || !targetValid) {
            std::cout << "Usage: yogi-ping [options] target" << std::endl;
            std::cout << std::endl;
            std::cout << "The target argument specifies the location of the Echoer terminals. Its syntax" << std::endl;
            std::cout << "is: \"[host:[port:]]/path\" (1) or \"[host:[port:]]target-hostname\" (2). The" << std::endl;
            std::cout << "defaults for host and port are 127.0.0.1 and 10000 respectively. Either an" << std::endl;
            std::cout << "absolute path (1) or the hostname of the target machine that is running the" << std::endl;
            std::cout << "Echoer (2) can be specified. The latter gets converted to the path" << std::endl;
            std::cout << "/Echoers/target-hostname." << std::endl;
            std::cout << std::endl;
            std::cout << desc << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "    yogi-ping 192.168.1.40:24001:/Echoers/elephant" << std::endl;
            std::cout << "    yogi-ping 192.168.1.40:elephant" << std::endl;
            std::cout << "    yogi-ping -i 0.5 elephant" << std::endl;
            return 1;
        }

        auto intervalInUs = static_cast<long>(interval * 1000000);
        if (intervalInUs <= 0) {
            throw std::runtime_error("Invalid interval");
        }

        Pinger pinger(std::chrono::microseconds(intervalInUs), count, payloadSize,
            useServiceTerminals, host, port, echoerLocation);
        pinger.run();
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
