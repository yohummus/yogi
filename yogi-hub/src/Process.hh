#ifndef PROCESS_HH
#define PROCESS_HH

#include "session_services/KnownTerminalsService.hh"

#include <QCoreApplication>

#include <yogi.hpp>

#include <list>


class Process : public QObject
{
    Q_OBJECT

public:
    Process(int argc, char* argv[]);

    int exec();

private:
    yogi::Configuration                     m_config;
    yogi::ProcessInterface                  m_pi;
    yogi::Node                              m_node;
    session_services::KnownTerminalsService m_kts;
    yogi::LocalConnection                   m_piConnection;
    QCoreApplication                        m_app;
    std::list<std::shared_ptr<QObject>>     m_services;

    void setup_scheduler();
    void setup_app();
    void setup_services();
    void setup_http_servers();
    void setup_ws_servers();

    static yogi::Configuration& resolve_hostname_in_location(yogi::Configuration& config);

    template <typename Service, typename... Args>
    std::shared_ptr<Service> add_service(Args&&... args);

    template <typename Service, typename... Args>
    std::shared_ptr<Service> add_service_if_enabled(const char* configPath, Args&&... args);

    template <typename Service, typename... Args>
    std::vector<std::shared_ptr<Service>> add_services_if_enabled(const char* configPath, Args&&... args);
};

#endif
