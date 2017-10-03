#include "Process.hh"
#include "web_servers/HttpServer.hh"
#include "web_servers/WebSocketServer.hh"
#include "yogi_network/YogiTcpClient.hh"
#include "yogi_network/YogiTcpServer.hh"
#include "testing/TestService.hh"
#include "http_services/ProtoCompilerService.hh"
#include "http_services/QueryService.hh"
#include "http_services/FileService.hh"
#include "session_services/AccountService.hh"
#include "session_services/CustomCommandService.hh"
#include "session_services/ConnectionsService.hh"

#include <boost/algorithm/string/replace.hpp>

#include <csignal>
#include <unistd.h>
#include <limits.h>

using namespace std::string_literals;


Process::Process(int argc, char* argv[])
: m_config(argc, argv, true)
, m_pi(resolve_hostname_in_location(m_config))
, m_node(m_pi.scheduler())
, m_kts(m_node) // create before connection for not missing the local ProcessInterface Terminals
, m_piConnection(m_pi.leaf(), m_node)
, m_app(argc, argv)
{
    yogi::Logger::set_thread_name("main");

    setup_scheduler();
    setup_app();
    setup_services();
}

int Process::exec()
{
    auto exitCode = m_app.exec();

    YOGI_LOG_INFO("Shutting down with exit code " << exitCode << "...");
    session_services::AccountService::destroy();
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
    add_service<testing::TestService>(m_node);

    setup_http_servers();
    setup_ws_servers();
}

void Process::setup_http_servers()
{
    auto protocService = add_service_if_enabled<http_services::ProtoCompilerService>("proto-compiler");

    auto servers = add_services_if_enabled<web_servers::HttpServer>("http-servers");
    for (auto& server : servers) {
        if (protocService) {
            server->add_service(protocService, "/compile");
        }

        auto queryService = add_service<http_services::QueryService>(server->config());
        server->add_service(queryService, "/query");

        auto fileService = add_service<http_services::FileService>(server->config());
        server->add_service(fileService, "/");

        server->start();
    }
}

void Process::setup_ws_servers()
{
    auto yogiServers = add_services_if_enabled<yogi_network::YogiTcpServer>("yogi-tcp-servers", m_node);
    auto yogiClients = add_services_if_enabled<yogi_network::YogiTcpClient>("yogi-tcp-clients", m_node);

    session_services::CustomCommandService::extract_command_details_from_config();
    session_services::ConnectionsService::register_factories(
        {yogiServers.begin(), yogiServers.end()},
        {yogiClients.begin(), yogiClients.end()}
    );

    if (m_config.get<bool>("account-service.enabled")) {
        session_services::AccountService::init();
    }

    auto servers = add_services_if_enabled<web_servers::WebSocketServer>("ws-servers", m_node);

    for (auto& server : servers) {
        server->start();
    };
}

yogi::Configuration& Process::resolve_hostname_in_location(yogi::Configuration& config)
{
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    auto location = config.location().to_string();
    boost::replace_all(location, "${HOSTNAME}", hostname);

    config.update("{\"yogi\": {\"location\": \""s + location + "\"}}");

    return config;
}

template <typename Service, typename... Args>
std::shared_ptr<Service> Process::add_service(Args&&... args)
{
    auto ptr = std::make_shared<Service>(std::forward<Args>(args)...);
    m_services.emplace_back(ptr);
    return ptr;
}

template <typename Service, typename... Args>
std::shared_ptr<Service> Process::add_service_if_enabled(const char* configPath, Args&&... args)
{
    if (m_pi.config().get<bool>(std::string(configPath) + ".enabled")) {
        return add_service<Service>(std::forward<Args>(args)...);
    }
    else {
        return {};
    }
}

template <typename Service, typename... Args>
std::vector<std::shared_ptr<Service>> Process::add_services_if_enabled(const char* configPath, Args&&... args)
{
    std::vector<std::shared_ptr<Service>> services;

    for (auto child : m_pi.config().get_child(configPath)) {
        if (child.second.get<bool>("enabled")) {
            auto service = add_service<Service>(child.second, std::forward<Args>(args)...);
            services.push_back(service);
        }
    }

    return services;
}
