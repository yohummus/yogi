#include "Process.hh"


int main(int argc, char *argv[])
{
    try {
        Process process(argc, argv);
        return process.exec();
    }
    catch (const std::exception& e) {
        YOGI_LOG_FATAL(e.what());
        return 1;
    }
}
