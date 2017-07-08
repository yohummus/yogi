#ifndef SESSION_SERVICES_TIMESERVICE_HH
#define SESSION_SERVICES_TIMESERVICE_HH

#include "Service.hh"
#include "../yogi_network/YogiSession.hh"


namespace session_services {

class TimeService : public Service
{
    Q_OBJECT

public:
    TimeService(yogi_network::YogiSession& session);

    virtual request_handlers_map make_request_handlers() override;

private:
    response_pair handle_current_time_request(QByteArray* request);
};

} // namespace session_services

#endif // SESSION_SERVICES_TIMESERVICE_HH
