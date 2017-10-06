#ifndef YOGI_NETWORK_YOGISESSION_HH
#define YOGI_NETWORK_YOGISESSION_HH

#include "../session_services/Service.hh"

#include <yogi.hpp>

#include <QVector>
#include <QMap>
#include <QWebSocket>
#include <QMetaObject>
#include <QMutex>

#include <chrono>
#include <unordered_map>
#include <vector>


namespace yogi_network {

class YogiSession : public QObject
{
    Q_OBJECT
    struct TerminalInfo;

public:
    YogiSession(QWebSocket* socket, yogi::Node& node, const QString& clientIdentification, QObject* parent);
    ~YogiSession();

    yogi::Node& node();
    yogi::Leaf& leaf();
    const QWebSocket& socket() const;

    QByteArray handle_request(QByteArray* request);
    void notify_client(session_services::Service::response_type notificationType, const QByteArray& data);

Q_SIGNALS:
    void notify_client(QWebSocket*, QByteArray);

private:
    yogi::Logger                                            m_logger;
    const QString                                           m_logPrefix;
    const QString                                           m_clientIdentification;
    QWebSocket*                                             m_socket;
    yogi::Node&                                             m_node;
    yogi::Leaf                                              m_leaf;
    yogi::LocalConnection                                   m_connection;
    std::vector<session_services::service_ptr>              m_services;
    std::vector<session_services::Service::request_handler> m_requestHandlerLut;

    static QByteArray make_response(session_services::Service::response_type status = session_services::Service::RES_OK);
    template <typename Service> void add_service();

    QByteArray handle_test_command(const QByteArray& request);
};

} // namespace yogi_network

#endif // YOGI_NETWORK_YOGISESSION_HH
