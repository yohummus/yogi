#ifndef SESSION_SERVICES_DNSSERVICE_HH
#define SESSION_SERVICES_DNSSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"

#include <QHostInfo>


namespace session_services {

class DnsService : public Service
{
    Q_OBJECT

public:
    DnsService(yogi_network::YogiSession& session);

    virtual request_handlers_map make_request_handlers() override;

private:
    yogi_network::YogiSession& m_session;

    response_pair handle_client_address_request(QByteArray* request);
    response_pair handle_start_dns_lookup_request(QByteArray* request);

private Q_SLOTS:
    void on_dns_lookup_finished(QHostInfo info);
};

} // namespace session_services

#endif // SESSION_SERVICES_DNSSERVICE_HH
