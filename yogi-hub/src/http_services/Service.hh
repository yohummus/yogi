#ifndef HTTP_SERVICES_SERVICE_HH
#define HTTP_SERVICES_SERVICE_HH

#include <QObject>
#include <QString>
#include <QMap>
#include <QByteArray>

#include <memory>
#include <functional>
#include <ostream>


namespace http_services {

enum request_type {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_OPTIONS,
    HTTP_DELETE,
    HTTP_CONNECT
};

enum status_code {
    HTTP_200 = 200,
    HTTP_201 = 201,
    HTTP_400 = 400,
    HTTP_403 = 403,
    HTTP_404 = 404,
    HTTP_500 = 500
};

class Service : public QObject
{
    Q_OBJECT

public:
    typedef std::function<void (status_code, const QByteArray& content, const QString& contentType,
        bool compressed)> completion_handler;

    virtual ~Service();

    virtual void async_handle_request(request_type, const QString& path,
        const QMap<QString, QString>& header, const QByteArray& content, completion_handler) =0;
};

typedef std::shared_ptr<Service> service_ptr;

} // namespace http_services

std::ostream& operator<< (std::ostream& os, http_services::request_type requestType);

#endif // HTTP_SERVICES_SERVICE_HH
