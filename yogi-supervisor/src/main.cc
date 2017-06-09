#include <yogi.hpp>

#include "Supervisor.hh"


int main(int argc, const char* argv[])
{
    try {
        yogi::ProcessInterface pi(argc, argv);

        Supervisor Supervisor;
        Supervisor.run();
    }
    catch (const std::exception& e) {
        YOGI_LOG_FATAL("Error: " << e.what());
        return 1;
    }

    YOGI_LOG_INFO("Shutting down...");
    return 0;
}
