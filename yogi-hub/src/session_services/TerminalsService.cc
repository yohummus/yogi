#include "TerminalsService.hh"
#include "../helpers/to_byte_array.hh"
#include "../helpers/read_from_stream.hh"
#include "../helpers/ostream.hh"
#include "../helpers/time.hh"

#include <QDataStream>

using namespace std::string_literals;


namespace session_services {

TerminalsService::TerminalsService(yogi_network::YogiSession& session)
: m_session(session)
, m_logger("Terminals Service")
{
}

TerminalsService::~TerminalsService()
{
}

TerminalsService::request_handlers_map TerminalsService::make_request_handlers()
{
    return {{
        REQ_CREATE_TERMINAL, [this](auto& request) {
            return this->handle_create_terminal_request(request);
        }}, {
        REQ_DESTROY_TERMINAL, [this](auto& request) {
            return this->handle_destroy_terminal_request(request);
        }}, {
        REQ_CREATE_BINDING, [this](auto& request) {
            return this->handle_create_binding_request(request);
        }}, {
        REQ_DESTROY_BINDING, [this](auto& request) {
            return this->handle_destroy_binding_request(request);
        }}, {
        REQ_MONITOR_BINDING_STATE, [this](auto& request) {
            return this->handle_monitor_binding_state_request(request);
        }}, {
        REQ_MONITOR_BUILTIN_BINDING_STATE, [this](auto& request) {
            return this->handle_monitor_builtin_binding_state_request(request);
        }}, {
        REQ_MONITOR_SUBSCRIPTION_STATE, [this](auto& request) {
            return this->handle_monitor_subscription_state_request(request);
        }}, {
        REQ_PUBLISH_MESSAGE, [this](auto& request) {
            return this->handle_publish_message_request(request);
        }}, {
        REQ_MONITOR_RECEIVED_PUBLISH_MESSAGES, [this](auto& request) {
            return this->handle_monitor_received_publish_messages_request(request);
        }}, {
        REQ_SCATTER_GATHER, [this](auto& request) {
            return this->handle_scatter_gather_request(request);
        }}, {
        REQ_MONITOR_RECEIVED_SCATTER_MESSAGES, [this](auto& request) {
            return this->handle_monitor_received_scatter_messages_request(request);
        }}, {
        REQ_RESPOND_TO_SCATTERED_MESSAGE, [this](auto& request) {
            return this->handle_respond_to_scattered_message_request(request);
        }}, {
        REQ_IGNORE_SCATTERED_MESSAGE, [this](auto& request) {
            return this->handle_ignore_scattered_message_request(request);
        }}
    };
}

TerminalsService::TerminalInfo::TerminalInfo(yogi::Leaf& leaf, yogi::terminal_type type,
    const char* name, yogi::Signature signature)
: terminal(yogi::Terminal::make_raw_terminal(leaf, type, name, signature))
{
}

TerminalsService::BindingInfo::BindingInfo(yogi::PrimitiveTerminal& terminal, const char* targets)
: binding(terminal, targets)
{
}

TerminalsService::response_pair TerminalsService::handle_create_terminal_request(
    const QByteArray& request)
{
    if (request.size() < 7 || request.at(request.size() - 1) != '\0') {
        return {RES_INVALID_REQUEST, {}};
    }

    QDataStream stream(request);
    stream.skipRawData(1);

    auto type      = helpers::read_from_stream<yogi::terminal_type>(&stream);
    auto signature = helpers::read_from_stream<yogi::Signature>(&stream);
    auto name      = (request.constData() + 6);

    if (type > yogi::CLIENT) {
        return {RES_INVALID_TERMINAL_TYPE, {}};
    }

    YOGI_LOG_DEBUG(m_logger, "Creating " << type << " Terminal '" << name
        << "' [" << signature << "]...");

    try {
        auto info = std::make_shared<TerminalInfo>(m_session.leaf(), type, name, signature);
        auto id = m_terminals.add(info);
        info->id = id;

        return {RES_OK, helpers::to_byte_array(id)};
    }
    catch (const yogi::Failure& e) {
        YOGI_LOG_WARNING("Could not create " << type << " Terminal '" << name
            << "' [" << signature << "]: " << e.what());

        return {RES_API_ERROR, helpers::to_byte_array(e)};
    }
}

TerminalsService::response_pair TerminalsService::handle_destroy_terminal_request(
    const QByteArray& request)
{
    QDataStream stream(request);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto info = m_terminals.take(id);
    if (!info) {
        return {RES_INVALID_TERMINAL_ID, {}};
    }

    return {RES_OK, {}};
}

TerminalsService::response_pair TerminalsService::handle_create_binding_request(
    const QByteArray& request)
{
    if (request.size() < 7 || request.at(request.size() - 1) != '\0') {
        return {RES_INVALID_REQUEST, {}};
    }

    QDataStream stream(request);
    stream.skipRawData(1);

    auto terminalId = helpers::read_from_stream<unsigned>(&stream);
    auto targets = request.constData() + 5;
    auto tmInfo = m_terminals.get(terminalId);
    if (!tmInfo) {
        return {RES_INVALID_TERMINAL_ID, {}};
    }

    if (!dynamic_cast<yogi::PrimitiveTerminal*>(tmInfo->terminal.get())) {
        return {RES_INVALID_TERMINAL_TYPE, {}};
    }

    YOGI_LOG_DEBUG(m_logger, "Creating Binding from " << tmInfo->terminal->type()
        << " Terminal '" << tmInfo->terminal->name() << "' ["
        << tmInfo->terminal->signature() << "] to '" << targets << "'...");

    try {
        auto bdInfo = std::make_shared<BindingInfo>(*static_cast<yogi::PrimitiveTerminal*>(
            tmInfo->terminal.get()), targets);

        auto bindingId = m_bindings.add(bdInfo);
        bdInfo->id = bindingId;

        return {RES_OK, helpers::to_byte_array(bindingId)};
    }
    catch (const yogi::Failure& e) {
        YOGI_LOG_WARNING("Could not create Binding from " << tmInfo->terminal->type()
            << " Terminal '" << tmInfo->terminal->name() << "' ["
            << tmInfo->terminal->signature() << "] to '" << targets << "': " << e.what());

        return {RES_API_ERROR, helpers::to_byte_array(e)};
    }
}

TerminalsService::response_pair TerminalsService::handle_destroy_binding_request(
    const QByteArray& request)
{
    QDataStream stream(request);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto info = m_bindings.take(id);
    if (!info) {
        return {RES_INVALID_BINDING_ID, {}};
    }

    return {RES_OK, {}};
}

TerminalsService::response_pair TerminalsService::handle_monitor_binding_state_request(
    const QByteArray& request)
{
    QDataStream stream(request);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto info = m_bindings.get(id);
    if (!info) {
        return {RES_INVALID_BINDING_ID, {}};
    }

    if (info->observer) {
        return {RES_ALREADY_MONITORING, {}};
    }

    info->observer = std::make_unique<yogi::BindingObserver>(info->binding);
    info->observer->add([this, info=info.get()](auto state) {
        this->on_binding_state_changed(*info, state);
    });
    info->observer->start();

    return {RES_OK, {}};
}

TerminalsService::response_pair TerminalsService::handle_monitor_builtin_binding_state_request(
    const QByteArray& request)
{
	QDataStream stream(request);
	stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto info = m_terminals.get(id);
    if (!info) {
        return {RES_INVALID_TERMINAL_ID, {}};
    }

    if (info->bindingObserver) {
        return {RES_ALREADY_MONITORING, {}};
    }

    if (!dynamic_cast<yogi::Binder*>(info->terminal.get())) {
        return {RES_INVALID_TERMINAL_TYPE, {}};
    }

    info->bindingObserver = std::make_unique<yogi::BindingObserver>(
        dynamic_cast<yogi::Binder&>(*info->terminal));
    info->bindingObserver->add([this, info=info.get()](auto state) {
        this->on_builtin_binding_state_changed(*info, state);
    });
    info->bindingObserver->start();

	return {RES_OK, {}};
}

TerminalsService::response_pair TerminalsService::handle_monitor_subscription_state_request(
    const QByteArray& request)
{
    QDataStream stream(request);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto info = m_terminals.get(id);
    if (!info) {
        return {RES_INVALID_TERMINAL_ID, {}};
    }

    if (info->subscriptionObserver) {
        return {RES_ALREADY_MONITORING, {}};
    }

    if (!dynamic_cast<yogi::Subscribable*>(info->terminal.get())) {
        return {RES_INVALID_TERMINAL_TYPE, {}};
    }

    info->subscriptionObserver = std::make_unique<yogi::SubscriptionObserver>(
        dynamic_cast<yogi::Subscribable&>(*info->terminal));
    info->subscriptionObserver->add([this, info=info.get()](auto state) {
        this->on_subscription_state_changed(*info, state);
    });
    info->subscriptionObserver->start();

    return {RES_OK, {}};
}

TerminalsService::response_pair TerminalsService::handle_publish_message_request(const QByteArray& request)
{
    return use_terminal(request, [&](TerminalInfo& info, QDataStream&) -> response_pair {
        const void*  data = request.data() + 5;
        const size_t size = request.size() - 5;

        switch (info.terminal->type()) {
        case yogi::terminal_type::PUBLISH_SUBSCRIBE:
            static_cast<yogi::RawPublishSubscribeTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::CACHED_PUBLISH_SUBSCRIBE:
            static_cast<yogi::RawCachedPublishSubscribeTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::PRODUCER:
            static_cast<yogi::RawProducerTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::CACHED_PRODUCER:
            static_cast<yogi::RawCachedProducerTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::MASTER:
            static_cast<yogi::RawMasterTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::CACHED_MASTER:
            static_cast<yogi::RawCachedMasterTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::SLAVE:
            static_cast<yogi::RawSlaveTerminal&>(*info.terminal).publish(data, size);
            break;

        case yogi::terminal_type::CACHED_SLAVE:
            static_cast<yogi::RawCachedSlaveTerminal&>(*info.terminal).publish(data, size);
            break;

        default:
            return {RES_INVALID_TERMINAL_TYPE, {}};
        }

        return {RES_OK, {}};
    });
}

TerminalsService::response_pair TerminalsService::handle_monitor_received_publish_messages_request(
    const QByteArray& request)
{
    return use_terminal(request, [&](TerminalInfo& info, QDataStream&) -> response_pair {
        const void*  data = request.data() + 5;
        const size_t size = request.size() - 5;

        bool hasCache = false;
        yogi::Optional<byte_array> cachedMessage;

        switch (info.terminal->type()) {
        case yogi::terminal_type::PUBLISH_SUBSCRIBE:
            create_message_observer_and_add_callback<yogi::RawPublishSubscribeTerminal>(info,
                &TerminalsService::on_published_message_received);
            break;

        case yogi::terminal_type::CACHED_PUBLISH_SUBSCRIBE:
            create_message_observer_and_add_callback<yogi::RawCachedPublishSubscribeTerminal>(info,
                &TerminalsService::on_cached_published_message_received);
            hasCache = true;
            cachedMessage = get_cached_message<yogi::RawCachedPublishSubscribeTerminal>(info);
            break;

        case yogi::terminal_type::CONSUMER:
            create_message_observer_and_add_callback<yogi::RawConsumerTerminal>(info,
                &TerminalsService::on_published_message_received);
            break;

        case yogi::terminal_type::CACHED_CONSUMER:
            create_message_observer_and_add_callback<yogi::RawCachedConsumerTerminal>(info,
                &TerminalsService::on_cached_published_message_received);
            hasCache = true;
            cachedMessage = get_cached_message<yogi::RawCachedConsumerTerminal>(info);
            break;

        case yogi::terminal_type::MASTER:
            create_message_observer_and_add_callback<yogi::RawMasterTerminal>(info,
                &TerminalsService::on_published_message_received);
            break;

        case yogi::terminal_type::CACHED_MASTER:
            create_message_observer_and_add_callback<yogi::RawCachedMasterTerminal>(info,
                &TerminalsService::on_cached_published_message_received);
            hasCache = true;
            cachedMessage = get_cached_message<yogi::RawCachedMasterTerminal>(info);
            break;

        case yogi::terminal_type::SLAVE:
            create_message_observer_and_add_callback<yogi::RawSlaveTerminal>(info,
                &TerminalsService::on_published_message_received);
            break;

        case yogi::terminal_type::CACHED_SLAVE:
            create_message_observer_and_add_callback<yogi::RawCachedSlaveTerminal>(info,
                &TerminalsService::on_cached_published_message_received);
            hasCache = true;
            cachedMessage = get_cached_message<yogi::RawCachedSlaveTerminal>(info);
            break;

        default:
            return {RES_INVALID_TERMINAL_TYPE, {}};
        }

        auto response = response_pair(RES_OK, {});
        if (hasCache) {
            response.second.append(helpers::to_byte_array(cachedMessage.has_value()));
            if (cachedMessage.has_value()) {
                response.second.append(cachedMessage->data(), static_cast<int>(cachedMessage->size()));
            }
        }

        return response;
    });
}

TerminalsService::response_pair TerminalsService::handle_scatter_gather_request(
    const QByteArray& request)
{
    return use_terminal(request, [&](TerminalInfo& info, QDataStream&) -> response_pair {
        yogi::raw_operation_id opId;

        switch (info.terminal->type()) {
        case yogi::terminal_type::SCATTER_GATHER: {{
            auto op = static_cast<yogi::RawScatterGatherTerminal&>(*info.terminal)
                .async_scatter_gather(request.data() + 5, static_cast<std::size_t>(request.size() - 5),
                    [&](auto& res, auto&& msg) {
                        return this->on_gathered_message_received(info, res, std::move(msg));
                    }
                );

            opId = op.operation_id();
            info.sgOperations.emplace(std::make_pair(opId, std::move(op)));
            }}
            break;

        case yogi::terminal_type::CLIENT: {{
            auto op = static_cast<yogi::RawClientTerminal&>(*info.terminal)
                .async_request(request.data() + 5, static_cast<std::size_t>(request.size() - 5),
                    [&](auto& res, auto&& msg) {
                        return this->on_response_received(info, res, std::move(msg));
                    }
                );

            opId = op.operation_id();
            info.scOperations.emplace(std::make_pair(opId, std::move(op)));
            }}
            break;

        default:
            return {RES_INVALID_TERMINAL_TYPE, {}};
        }

        return {RES_OK, helpers::to_byte_array(opId)};
    });
}

TerminalsService::response_pair TerminalsService::handle_monitor_received_scatter_messages_request(
    const QByteArray& request)
{
    return use_terminal(request, [&](TerminalInfo& info, QDataStream&) -> response_pair {
        switch (info.terminal->type()) {
        case yogi::terminal_type::SCATTER_GATHER:
            create_message_observer_and_add_callback<yogi::RawScatterGatherTerminal>(info,
                &TerminalsService::on_scattered_message_received);
            break;

        case yogi::terminal_type::SERVICE:
            create_message_observer_and_add_callback<yogi::RawServiceTerminal>(info,
                &TerminalsService::on_request_received);
            break;

        default:
            return {RES_INVALID_TERMINAL_TYPE, {}};
        }

        return {RES_OK, {}};
    });
}

TerminalsService::response_pair TerminalsService::handle_respond_to_scattered_message_request(
    const QByteArray& request)
{
    return use_terminal(request, [&](TerminalInfo& info, QDataStream& stream) -> response_pair {
        auto opId = helpers::read_from_stream<yogi::raw_operation_id>(&stream);

        switch (info.terminal->type()) {
        case yogi::terminal_type::SCATTER_GATHER:
            return respond_to_scattered_message(info.sgScatteredMessages, opId, request);

        case yogi::terminal_type::SERVICE:
            return respond_to_scattered_message(info.scRequests, opId, request);

        default:
            return {RES_INVALID_TERMINAL_TYPE, {}};
        }
    });
}

TerminalsService::response_pair TerminalsService::handle_ignore_scattered_message_request(
    const QByteArray& request)
{
    return use_terminal(request, [&](TerminalInfo& info, QDataStream& stream) -> response_pair {
        auto opId = helpers::read_from_stream<yogi::raw_operation_id>(&stream);

        switch (info.terminal->type()) {
        case yogi::terminal_type::SCATTER_GATHER:
            return ignore_scattered_message(info.sgScatteredMessages, opId);

        case yogi::terminal_type::SERVICE:
            return ignore_scattered_message(info.scRequests, opId);

        default:
            return {RES_INVALID_TERMINAL_TYPE, {}};
        }
    });
}

void TerminalsService::on_binding_state_changed(BindingInfo& info, yogi::binding_state state)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(state);

    m_session.notify_client(MON_BINDING_STATE_CHANGED, data);
}

void TerminalsService::on_builtin_binding_state_changed(TerminalInfo& info, yogi::binding_state state)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(state);

    m_session.notify_client(MON_BUILTIN_BINDING_STATE_CHANGED, data);
}

void TerminalsService::on_subscription_state_changed(TerminalInfo& info, yogi::subscription_state state)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(state);

    m_session.notify_client(MON_SUBSCRIPTION_STATE_CHANGED, data);
}

void TerminalsService::on_published_message_received(TerminalInfo& info, const byte_array& payload, yogi::cached_flag cached)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data.append(payload.data(), static_cast<int>(payload.size()));

    m_session.notify_client(MON_PUBLISHED_MESSAGE_RECEIVED, data);
}

void TerminalsService::on_cached_published_message_received(TerminalInfo& info, const byte_array& payload, yogi::cached_flag cached)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(cached);
    data.append(payload.data(), static_cast<int>(payload.size()));

    m_session.notify_client(MON_CACHED_PUBLISHED_MESSAGE_RECEIVED, data);
}

void TerminalsService::on_scattered_message_received(TerminalInfo& info, yogi::RawScatterGatherTerminal::ScatteredMessage&& msg)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(msg.operation_id());
    data.append(msg.data().data(), static_cast<int>(msg.data().size()));

    std::lock_guard<std::mutex> lock(m_mapsMutex);
    info.sgScatteredMessages.emplace(std::make_pair(msg.operation_id(), std::move(msg)));

    m_session.notify_client(MON_SCATTERED_MESSAGE_RECEIVED, data);
}

yogi::control_flow TerminalsService::on_gathered_message_received(TerminalInfo& info, const yogi::Result& result, yogi::RawScatterGatherTerminal::GatheredMessage&& msg)
{
    if (!result) {
        return yogi::control_flow::STOP;
    }

    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(msg.operation_id());
    data += helpers::to_byte_array(msg.flags());
    data.append(msg.data().data(), static_cast<int>(msg.data().size()));

    if (msg.flags() & yogi::gather_flags::FINISHED) {
        std::lock_guard<std::mutex> lock(m_mapsMutex);
        info.sgOperations.erase(msg.operation_id());
    }

    m_session.notify_client(MON_GATHERED_MESSAGE_RECEIVED, data);

    return yogi::control_flow::CONTINUE;
}

void TerminalsService::on_request_received(TerminalInfo& info, yogi::RawServiceTerminal::Request&& request)
{
    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(request.operation_id());
    data.append(request.data().data(), static_cast<int>(request.data().size()));

    {{
        std::lock_guard<std::mutex> lock(m_mapsMutex);
        info.scRequests.emplace(std::make_pair(request.operation_id(), std::move(request)));
    }}

    m_session.notify_client(MON_SCATTERED_MESSAGE_RECEIVED, data);
}

yogi::control_flow TerminalsService::on_response_received(TerminalInfo& info, const yogi::Result& result, yogi::RawClientTerminal::Response&& response)
{
    if (!result) {
        return yogi::control_flow::STOP;
    }

    QByteArray data;
    data += helpers::to_byte_array(info.id);
    data += helpers::to_byte_array(response.operation_id());
    data += helpers::to_byte_array(response.flags());
    data.append(response.data().data(), static_cast<int>(response.data().size()));

    if (response.flags() & yogi::gather_flags::FINISHED) {
        std::lock_guard<std::mutex> lock(m_mapsMutex);
        info.scOperations.erase(response.operation_id());
    }

    m_session.notify_client(MON_GATHERED_MESSAGE_RECEIVED, data);

    return yogi::control_flow::CONTINUE;
}

template <typename Fn>
TerminalsService::response_pair TerminalsService::use_terminal(const QByteArray& request, Fn fn)
{
    QDataStream stream(request);
    stream.skipRawData(1);

    auto id = helpers::read_from_stream<unsigned>(&stream);
    auto info = m_terminals.get(id);
    if (!info) {
        return {RES_INVALID_TERMINAL_ID, {}};
    }

    try {
        return fn(*info, stream);
    }
    catch(const yogi::Failure& failure) {
        return {RES_API_ERROR, QByteArray(failure.to_string().c_str())};
    }
}

template <typename Terminal>
void TerminalsService::create_message_observer_and_add_callback(TerminalInfo& info,
    void (TerminalsService::*fn)(TerminalInfo&, const byte_array&, yogi::cached_flag))
{
    auto observer = std::make_unique<yogi::MessageObserver<Terminal>>(static_cast<Terminal&>(*info.terminal));
    observer->add([&, fn](auto& payload, auto cached) {
        (this->*fn)(info, payload, cached);
    });

    info.messageObserver = std::move(observer);
    info.messageObserver->start();
}

template <typename Terminal, typename ScatteredMessage>
void TerminalsService::create_message_observer_and_add_callback(TerminalInfo& info,
    void (TerminalsService::*fn)(TerminalInfo&, ScatteredMessage&&))
{
    auto observer = std::make_unique<yogi::MessageObserver<Terminal>>(static_cast<Terminal&>(*info.terminal));
    observer->set([&, fn](auto&& msg) {
        (this->*fn)(info, std::move(msg));
    });

    info.messageObserver = std::move(observer);
    info.messageObserver->start();
}

template <typename Terminal>
yogi::Optional<TerminalsService::byte_array> TerminalsService::get_cached_message(TerminalInfo& info)
{
    auto& terminal = static_cast<Terminal&>(*info.terminal);

    try {
        return {terminal.get_cached_message()};
    }
    catch (const yogi::Failure&) {
        return {};
    }
}

template <typename MessageMap>
TerminalsService::response_pair TerminalsService::respond_to_scattered_message(MessageMap& messages,
    yogi::raw_operation_id opId, const QByteArray& request)
{
    std::lock_guard<std::mutex> lock(m_mapsMutex);

    auto it = messages.find(opId);
    if (it == messages.end()) {
        return {RES_INVALID_OPERATION_ID, {}};
    }

    it->second.respond(request.data() + 9, request.size() - 9);
    messages.erase(it);

    return {RES_OK, {}};
}

template <typename MessageMap>
TerminalsService::response_pair TerminalsService::ignore_scattered_message(MessageMap& messages,
    yogi::raw_operation_id opId)
{
    std::lock_guard<std::mutex> lock(m_mapsMutex);

    auto it = messages.find(opId);
    if (it == messages.end()) {
        return {RES_INVALID_OPERATION_ID, {}};
    }

    it->second.ignore();
    messages.erase(it);

    return {RES_OK, {}};
}

} // namespace session_services
