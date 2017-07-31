#include <yogi.hpp>

#include "Echoer.hh"


int main(int argc, const char* argv[])
{
    yogi::Logger::set_thread_name("main");

    try {
        yogi::ProcessInterface pi(argc, argv, true);

        Echoer echoer(pi.config());
        echoer.run();
    }
    catch (const std::exception& e) {
        YOGI_LOG_FATAL(e.what());
        return 1;
    }

    YOGI_LOG_INFO("Shutting down...");
    return 0;
}
