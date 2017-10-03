#include "YogiSession.hh"
#include "../testing/TestService.hh"
#include "../helpers/ostream.hh"
#include "../helpers/read_from_stream.hh"
#include "../helpers/to_byte_array.hh"
#include "../session_services/DnsService.hh"
#include "../session_services/KnownTerminalsService.hh"
#include "../session_services/CustomCommandService.hh"
#include "../session_services/TimeService.hh"
#include "../session_services/VersionService.hh"
#include "../session_services/ConnectionsService.hh"
#include "../session_services/TerminalsService.hh"
#include "../session_services/AccountService.hh"

#include <QtDebug>
#include <QDataStream>
#include <QIODevice>

#include <cassert>
#include <memory>
using namespace std::string_literals;

#define TERMINAL_INFO_BUFFER_SIZE 1024
#define MESSAGE_BUFFER_SIZE 32 * 1024


namespace yogi_network {

YogiSession::YogiSession(QWebSocket* socket, yogi::Node& node, const QString& clientIdentification, QObject* parent)
: QObject(parent)
, m_logger("Yogi Session")
, m_logPrefix(clientIdentification + ':')
, m_clientIdentification(clientIdentification)
, m_socket(socket)
, m_node(node)
, m_leaf(m_node.scheduler())
, m_connection(m_node, m_leaf)
{
    YOGI_LOG_INFO(m_logger, "YOGI session for " << m_clientIdentification << " started");

    using namespace session_services;
    add_service<DnsService>();
    add_service<KnownTerminalsService>();
    add_service<CustomCommandService>();
    add_service<TimeService>();
    add_service<VersionService>();
    add_service<ConnectionsService>();
    add_service<TerminalsService>();

    if (yogi::ProcessInterface::config().get<bool>("account-service.enabled")) {
        add_service<AccountService>();
    }
}

YogiSession::~YogiSession()
{
    m_services.clear();

    YOGI_LOG_INFO(m_logger, "YOGI session for " << m_clientIdentification << " destroyed");
}

yogi::Node& YogiSession::node()
{
    return m_node;
}

yogi::Leaf& YogiSession::leaf()
{
    return m_leaf;
}

const QWebSocket& YogiSession::socket() const
{
    return *m_socket;
}

QByteArray YogiSession::handle_request(QByteArray* request)
{
    try {
        if (!request->isEmpty()) {
            auto requestType = static_cast<session_services::Service::request_type>(request->at(0));

            if (requestType < m_requestHandlerLut.size() && m_requestHandlerLut[requestType]) {
                auto response = m_requestHandlerLut[requestType](*request);
                return make_response(response.first) + response.second;
            }

            switch (requestType) {
            case session_services::Service::REQ_TEST_COMMAND:
                return handle_test_command(*request);

            default:
                break;
            }
        }
    }
    catch (const yogi::Failure& failure) {
        YOGI_LOG_ERROR(m_logger, "Handling request " << request->toHex().data() << " failed: " << failure);
        return make_response(session_services::Service::RES_API_ERROR) + failure.what();
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(m_logger, "Handling request " << request->toHex().data() << " failed: " << e.what());
        return make_response(session_services::Service::RES_INTERNAL_SERVER_ERROR);
    }

    YOGI_LOG_ERROR(m_logger, "Invalid request: " << request->toHex().data());
    return make_response(session_services::Service::RES_INVALID_REQUEST);
}

void YogiSession::notify_client(session_services::Service::response_type notificationType, const QByteArray& data)
{
    emit(notify_client(m_socket, make_response(notificationType) + data));
}

QByteArray YogiSession::make_response(session_services::Service::response_type status)
{
    return QByteArray(1, status);
}

template <typename Service>
void YogiSession::add_service()
{
    auto service = std::make_shared<Service>(*this);
    m_services.push_back(service);

    for (auto& entry : service->make_request_handlers()) {
        auto requestType = entry.first;
        if (m_requestHandlerLut.size() < requestType + 1) {
            m_requestHandlerLut.resize(requestType + 1);
        }

        assert (!m_requestHandlerLut[requestType]);
        m_requestHandlerLut[requestType] = entry.second;
    }
}

QByteArray YogiSession::handle_test_command(const QByteArray& request)
{
    bool ok = testing::TestService::execute_command(request.mid(1));
    return make_response(ok ? session_services::Service::RES_OK : session_services::Service::RES_INVALID_REQUEST);
}

} // namespace yogi_network
