#ifndef HTTP_SERVICES_QUERYSERVICE_HH
#define HTTP_SERVICES_QUERYSERVICE_HH

#include "Service.hh"

#include <yogi.hpp>


namespace http_services {

class QueryService : public Service
{
public:
    QueryService(const yogi::ConfigurationChild& config);

    virtual void async_handle_request(request_type, const QString& path,
        const QMap<QString, QString>& header, const QByteArray& content, completion_handler) override;

private:
    yogi::Logger           m_logger;
    QMap<QString, QString> m_queries;
};

} // namespace http_services

#endif // HTTP_SERVICES_QUERYSERVICE_HH
