#include "HttpServer.hpp"
#include "WebSocketServer.hpp"
#include "TestService.hpp"
#include "YogiTcpClient.hpp"
#include "YogiTcpServer.hpp"
#include "KnownTerminalsMonitor.hpp"
#include "ProtoCompiler.hpp"
#include "CustomCommandService.hpp"

#include <yogi.hpp>

#include <QCoreApplication>
#include <QtDebug>
#include <QFile>
#include <csignal>


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    yogi::Logger::set_thread_name("main");

    try {
        yogi::ProcessInterface pi(argc, argv, true);

        pi.scheduler().set_thread_pool_size(pi.config().get<size_t>("performance.scheduler-thread-pool-size"));

        // setup YOGI
        yogi::Node node(pi.scheduler());

        // setup the services
        KnownTerminalsMonitor knownTerminalsMonitor(node);
        TestService testService(node);
        ProtoCompiler protoCompiler;
        CustomCommandService customCommandService;

        // setup YOGI connection factories
        std::vector<std::unique_ptr<YogiTcpServer>>  yogiTcpServers;
        for (auto child : pi.config().get_child("yogi-tcp-servers")) {
             yogiTcpServers.emplace_back(std::make_unique<YogiTcpServer>(child.second, node));
        }

        std::vector<std::unique_ptr<YogiTcpClient>>  yogiTcpClients;
        for (auto child : pi.config().get_child("yogi-tcp-clients")) {
             yogiTcpClients.emplace_back(std::make_unique<YogiTcpClient>(child.second, node));
        }

        // setup web servers
        std::vector<std::unique_ptr<HttpServer>> httpServers;
        for (auto child : pi.config().get_child("http-servers")) {
            httpServers.emplace_back(std::make_unique<HttpServer>(child.second));
        }

        std::vector<std::unique_ptr<WebSocketServer>> webSocketServers;
        for (auto child : pi.config().get_child("ws-servers")) {
            webSocketServers.emplace_back(std::make_unique<WebSocketServer>(child.second, node));
        }

        // install signal handlers
        signal(SIGINT, [](int) {
            YOGI_LOG_INFO("SIGINT received");
            QCoreApplication::exit(0);
        });

        signal(SIGTERM, [](int) {
            YOGI_LOG_INFO("SIGTERM received");
            QCoreApplication::exit(0);
        });

#ifdef _WIN32
        signal(SIGBREAK, [](int) {
            YOGI_LOG_INFO("SIGBREAK received");
            QCoreApplication::exit(0);
        });
#endif

        // start the main event loop
        int exitCode = app.exec();

        // cleanup
        YOGI_LOG_INFO("Shutting down with exit code " << exitCode << "...");
        return exitCode;
    }
    catch (const std::exception& e) {
        YOGI_LOG_FATAL(e.what());
        return 1;
    }
}
