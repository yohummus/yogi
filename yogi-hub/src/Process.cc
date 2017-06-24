#include "Process.hh"

#include <csignal>


Process::Process(int argc, char* argv[])
: m_pi(argc, argv, true)
, m_node(m_pi.scheduler())
, m_piConnection(m_pi.leaf(), m_node)
, m_app(argc, argv)
{
    yogi::Logger::set_thread_name("main");

    setup_scheduler();
    setup_app();
    setup_services();
    setup_yogi_servers_and_clients();
    setup_web_servers();
}

int Process::exec()
{
    auto exitCode = m_app.exec();

    YOGI_LOG_INFO("Shutting down with exit code " << exitCode << "...");
    return exitCode;
}

void Process::setup_scheduler()
{
    auto n = m_pi.config().get<size_t>("performance.scheduler-thread-pool-size");
    m_pi.scheduler().set_thread_pool_size(n);
}

void Process::setup_app()
{
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
}

void Process::setup_services()
{
    m_knownTerminalsMonitor = std::make_unique<yogi_network::KnownTerminalsMonitor>(m_node);
    m_testService           = std::make_unique<testing::TestService>(m_node);
    m_protoCompiler         = std::make_unique<protobuf::ProtoCompiler>();
    m_customCommandService  = std::make_unique<commands::CustomCommandService>();
}

void Process::setup_yogi_servers_and_clients()
{
    for (auto child : m_pi.config().get_child("yogi-tcp-servers")) {
        m_yogiServers.emplace_back(std::make_unique<yogi_network::YogiTcpServer>(child.second, m_node));
    }

    for (auto child : m_pi.config().get_child("yogi-tcp-clients")) {
        m_yogiClients.emplace_back(std::make_unique<yogi_network::YogiTcpClient>(child.second, m_node));
    }
}

void Process::setup_web_servers()
{
    for (auto child : m_pi.config().get_child("http-servers")) {
        m_httpServers.emplace_back(std::make_unique<web_servers::HttpServer>(child.second));
    }

    for (auto child : m_pi.config().get_child("ws-servers")) {
        m_wsServers.emplace_back(std::make_unique<web_servers::WebSocketServer>(child.second, m_node));
    }
}
