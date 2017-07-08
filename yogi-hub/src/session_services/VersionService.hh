#ifndef SESSION_SERVICES_VERSIONSERVICE_HH
#define SESSION_SERVICES_VERSIONSERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"


namespace session_services {

class VersionService : public Service
{
    Q_OBJECT

public:
    VersionService(yogi_network::YogiSession& session);

    virtual request_handlers_map make_request_handlers() override;

private:
    response_pair handle_version_request(QByteArray* request);
};

} // namespace session_services

#endif // SESSION_SERVICES_VERSIONSERVICE_HH
