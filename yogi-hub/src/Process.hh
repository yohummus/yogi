#ifndef PROCESS_HH
#define PROCESS_HH

#include "web_servers/HttpServer.hh"
#include "web_servers/WebSocketServer.hh"
#include "yogi_network/YogiTcpClient.hh"
#include "yogi_network/YogiTcpServer.hh"
#include "yogi_network/KnownTerminalsMonitor.hh"
#include "protobuf/ProtoCompiler.hh"
#include "commands/CustomCommandService.hh"
#include "testing/TestService.hh"

#include <QCoreApplication>

#include <yogi.hpp>


class Process
{
public:
    Process(int argc, char* argv[]);

    int exec();

private:
    template <typename T> using service_ptr = std::unique_ptr<T>;
    template <typename T> using service_vector = std::vector<service_ptr<T>>;

    yogi::ProcessInterface                           m_pi;
    yogi::Node                                       m_node;
    yogi::LocalConnection                            m_piConnection;
    QCoreApplication                                 m_app;

    service_ptr<yogi_network::KnownTerminalsMonitor> m_knownTerminalsMonitor;
    service_ptr<testing::TestService>                m_testService;
    service_ptr<protobuf::ProtoCompiler>             m_protoCompiler;
    service_ptr<commands::CustomCommandService>      m_customCommandService;
    service_vector<yogi_network::YogiTcpServer>      m_yogiServers;
    service_vector<yogi_network::YogiTcpClient>      m_yogiClients;
    service_vector<web_servers::HttpServer>          m_httpServers;
    service_vector<web_servers::WebSocketServer>     m_wsServers;

    void setup_scheduler();
    void setup_app();
    void setup_services();
    void setup_yogi_servers_and_clients();
    void setup_web_servers();
};

#endif
