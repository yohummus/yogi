#include "terminals.hpp"
#include "leaf.hpp"
#include "process.hpp"

#include <yogi_core.h>

#include <stdexcept>


namespace yogi {

std::unique_ptr<Terminal> Terminal::_make_raw_terminal(Leaf& leaf, terminal_type type, std::string name, Signature signature)
{
    switch (type) {
    case DEAF_MUTE:                return std::make_unique<RawDeafMuteTerminal>              (leaf, std::move(name), signature);
    case PUBLISH_SUBSCRIBE:        return std::make_unique<RawPublishSubscribeTerminal>      (leaf, std::move(name), signature);
    case SCATTER_GATHER:           return std::make_unique<RawScatterGatherTerminal>         (leaf, std::move(name), signature);
    case CACHED_PUBLISH_SUBSCRIBE: return std::make_unique<RawCachedPublishSubscribeTerminal>(leaf, std::move(name), signature);
    case PRODUCER:                 return std::make_unique<RawProducerTerminal>              (leaf, std::move(name), signature);
    case CONSUMER:                 return std::make_unique<RawConsumerTerminal>              (leaf, std::move(name), signature);
    case CACHED_PRODUCER:          return std::make_unique<RawCachedProducerTerminal>        (leaf, std::move(name), signature);
    case CACHED_CONSUMER:          return std::make_unique<RawCachedConsumerTerminal>        (leaf, std::move(name), signature);
    case MASTER:                   return std::make_unique<RawMasterTerminal>                (leaf, std::move(name), signature);
    case SLAVE:                    return std::make_unique<RawSlaveTerminal>                 (leaf, std::move(name), signature);
    case CACHED_MASTER:            return std::make_unique<RawCachedMasterTerminal>          (leaf, std::move(name), signature);
    case CACHED_SLAVE:             return std::make_unique<RawCachedSlaveTerminal>           (leaf, std::move(name), signature);
    case SERVICE:                  return std::make_unique<RawServiceTerminal>               (leaf, std::move(name), signature);
    case CLIENT:                   return std::make_unique<RawClientTerminal>                (leaf, std::move(name), signature);

    default:
        throw std::runtime_error("Invalid terminal type");
    }
}

std::unique_ptr<Terminal> Terminal::_make_raw_terminal(Leaf& leaf, terminal_type type, const Path& path, Signature signature)
{
    return _make_raw_terminal(leaf, type, path.to_string(), signature);
}

Terminal::Terminal(Leaf& leaf, int type, std::string name, Signature signature)
: Object(YOGI_CreateTerminal, leaf.handle(), type, name.c_str(), signature.raw())
, m_leaf(leaf)
, m_name(name)
, m_signature(signature)
{
}

Terminal::Terminal(Leaf& leaf, int type, const Path& path, Signature signature)
: Terminal(leaf, type, path.to_string(), signature)
{
}

Terminal::Terminal(int type, std::string name, Signature signature)
: Terminal(type, Path(name), signature)
{
}

Terminal::Terminal(int type, const Path& path, Signature signature)
: Terminal(ProcessInterface::leaf(), type, path.is_absolute() ? path : (ProcessInterface::location() / path), signature)
{
}

RawDeafMuteTerminal::~RawDeafMuteTerminal()
{
    this->_destroy();
}

const std::string& RawDeafMuteTerminal::class_name() const
{
    static std::string s = "RawDeafMuteTerminal";
    return s;
}

terminal_type RawDeafMuteTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

RawPublishSubscribeTerminal::~RawPublishSubscribeTerminal()
{
    this->_destroy();
}

const std::string& RawPublishSubscribeTerminal::class_name() const
{
    static std::string s = "RawPublishSubscribeTerminal";
    return s;
}

terminal_type RawPublishSubscribeTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawPublishSubscribeTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_PS_Publish, this, data, size);
}

void RawPublishSubscribeTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawPublishSubscribeTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_PS_Publish, this, data, size);
}

bool RawPublishSubscribeTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

void RawPublishSubscribeTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_PS_AsyncReceiveMessage, this, completionHandler);
}

void RawPublishSubscribeTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_PS_CancelReceiveMessage, this);
}

RawCachedPublishSubscribeTerminal::~RawCachedPublishSubscribeTerminal()
{
    this->_destroy();
}

const std::string& RawCachedPublishSubscribeTerminal::class_name() const
{
    static std::string s = "RawCachedPublishSubscribeTerminal";
    return s;
}

terminal_type RawCachedPublishSubscribeTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawCachedPublishSubscribeTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_CPS_Publish, this, data, size);
}

void RawCachedPublishSubscribeTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawCachedPublishSubscribeTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_CPS_Publish, this, data, size);
}

bool RawCachedPublishSubscribeTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

std::vector<char> RawCachedPublishSubscribeTerminal::get_cached_message()
{
    return internal::get_cached_raw_message(YOGI_CPS_GetCachedMessage, this);
}

void RawCachedPublishSubscribeTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_CPS_AsyncReceiveMessage, this, completionHandler);
}

void RawCachedPublishSubscribeTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_CPS_CancelReceiveMessage, this);
}

RawScatterGatherTerminal::Operation::Operation(RawScatterGatherTerminal& terminal, ::yogi::raw_operation_id operationId)
: m_terminal(&terminal)
, m_operationId(operationId)
{
}

RawScatterGatherTerminal::Operation::Operation(Operation&& other)
{
    *this = std::move(other);
}

RawScatterGatherTerminal::Operation& RawScatterGatherTerminal::Operation::operator= (Operation&& other)
{
    m_terminal    = other.m_terminal;
    m_operationId = other.m_operationId;
    return *this;
}

void RawScatterGatherTerminal::Operation::cancel()
{
    internal::cancel_operation(YOGI_SG_CancelScatterGather, m_terminal, m_operationId);
    m_operationId = 0;
}

RawScatterGatherTerminal::GatheredMessage::GatheredMessage(RawScatterGatherTerminal& terminal, ::yogi::raw_operation_id operationId, gather_flags flags, std::vector<char>&& data)
: m_terminal(terminal)
, m_operationId(operationId)
, m_flags(flags)
, m_data(std::move(data))
{
}

RawScatterGatherTerminal::ScatteredMessage::ScatteredMessage(RawScatterGatherTerminal& terminal, ::yogi::raw_operation_id operationId, std::vector<char>&& data)
: m_terminal(&terminal)
, m_operationId(operationId)
, m_data(std::move(data))
{
}

RawScatterGatherTerminal::ScatteredMessage& RawScatterGatherTerminal::ScatteredMessage::operator= (ScatteredMessage&& other)
{
    m_terminal    = other.m_terminal;
    m_operationId = other.m_operationId;
    m_data        = std::move(other.m_data);

    other.m_terminal = nullptr;

    return *this;
}

void RawScatterGatherTerminal::ScatteredMessage::respond(const void* data, std::size_t size)
{
    internal::respond_to_scattered_raw_message(YOGI_SG_RespondToScatteredMessage, m_terminal, m_operationId, data, size);
    m_operationId = 0;
}

bool RawScatterGatherTerminal::ScatteredMessage::try_respond(const void* data, std::size_t size)
{
    int res = internal::try_respond_to_scattered_raw_message(YOGI_SG_RespondToScatteredMessage, m_terminal, m_operationId, data, size);
    if (res == YOGI_OK) {
        m_operationId = 0;
    }

    return res == YOGI_OK;
}

void RawScatterGatherTerminal::ScatteredMessage::respond(const std::vector<char>& data)
{
    respond(data.data(), data.size());
}

bool RawScatterGatherTerminal::ScatteredMessage::try_respond(const std::vector<char>& data)
{
    return try_respond(data.data(), data.size());
}

void RawScatterGatherTerminal::ScatteredMessage::ignore()
{
    internal::ignore_scattered_message(YOGI_SG_IgnoreScatteredMessage, m_terminal, m_operationId);
    m_operationId = 0;
}

bool RawScatterGatherTerminal::ScatteredMessage::try_ignore()
{
    int res = internal::try_ignore_scattered_message(YOGI_SG_IgnoreScatteredMessage, m_terminal, m_operationId);
    if (res == YOGI_OK) {
        m_operationId = 0;
    }

    return res == YOGI_OK;
}

RawScatterGatherTerminal::~RawScatterGatherTerminal()
{
    this->_destroy();
}

const std::string& RawScatterGatherTerminal::class_name() const
{
    static std::string s = "RawScatterGatherTerminal";
    return s;
}

terminal_type RawScatterGatherTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

RawScatterGatherTerminal::Operation RawScatterGatherTerminal::async_scatter_gather(const void* scatterData, std::size_t scatterSize,
    std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler)
{
    return internal::async_scatter_gather_raw<Operation>(YOGI_SG_AsyncScatterGather, this, scatterData, scatterSize, completionHandler);
}

RawScatterGatherTerminal::Operation RawScatterGatherTerminal::async_scatter_gather(const std::vector<char>& data,
    std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler)
{
    return async_scatter_gather(data.data(), data.size(), completionHandler);
}

RawScatterGatherTerminal::Operation RawScatterGatherTerminal::try_async_scatter_gather(const void* scatterData, std::size_t scatterSize,
    std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler)
{
    try {
        return async_scatter_gather(scatterData, scatterSize, completionHandler);
    }
    catch (...) {
        return Operation(*this);
    }
}

RawScatterGatherTerminal::Operation RawScatterGatherTerminal::try_async_scatter_gather(const std::vector<char>& data,
    std::function<control_flow (const Result&, GatheredMessage&&)> completionHandler)
{
    return try_async_scatter_gather(data.data(), data.size(), completionHandler);
}

void RawScatterGatherTerminal::async_receive_scattered_message(std::function<void (const Result&, ScatteredMessage&&)> completionHandler)
{
    internal::async_receive_scattered_raw_message(YOGI_SG_AsyncReceiveScatteredMessage, this, completionHandler);
}

void RawScatterGatherTerminal::cancel_receive_scattered_message()
{
    internal::cancel(YOGI_SG_CancelReceiveScatteredMessage, this);
}

RawProducerTerminal::~RawProducerTerminal()
{
    this->_destroy();
}

const std::string& RawProducerTerminal::class_name() const
{
    static std::string s = "RawProducerTerminal";
    return s;
}

terminal_type RawProducerTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawProducerTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_PC_Publish, this, data, size);
}

void RawProducerTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawProducerTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_PC_Publish, this, data, size);
}

bool RawProducerTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

RawConsumerTerminal::~RawConsumerTerminal()
{
    this->_destroy();
}

const std::string& RawConsumerTerminal::class_name() const
{
    static std::string s = "RawConsumerTerminal";
    return s;
}

terminal_type RawConsumerTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawConsumerTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_PC_AsyncReceiveMessage, this, completionHandler);
}

void RawConsumerTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_PC_CancelReceiveMessage, this);
}

RawCachedProducerTerminal::~RawCachedProducerTerminal()
{
    this->_destroy();
}

const std::string& RawCachedProducerTerminal::class_name() const
{
    static std::string s = "RawCachedProducerTerminal";
    return s;
}

terminal_type RawCachedProducerTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawCachedProducerTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_CPC_Publish, this, data, size);
}

void RawCachedProducerTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawCachedProducerTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_CPC_Publish, this, data, size);
}

bool RawCachedProducerTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

RawCachedConsumerTerminal::~RawCachedConsumerTerminal()
{
    this->_destroy();
}

const std::string& RawCachedConsumerTerminal::class_name() const
{
    static std::string s = "RawCachedConsumerTerminal";
    return s;
}

terminal_type RawCachedConsumerTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

std::vector<char> RawCachedConsumerTerminal::get_cached_message()
{
    return internal::get_cached_raw_message(YOGI_CPC_GetCachedMessage, this);
}

void RawCachedConsumerTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_CPC_AsyncReceiveMessage, this, completionHandler);
}

void RawCachedConsumerTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_CPC_CancelReceiveMessage, this);
}

RawMasterTerminal::~RawMasterTerminal()
{
    this->_destroy();
}

const std::string& RawMasterTerminal::class_name() const
{
    static std::string s = "RawMasterTerminal";
    return s;
}

terminal_type RawMasterTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawMasterTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_MS_Publish, this, data, size);
}

void RawMasterTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawMasterTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_MS_Publish, this, data, size);
}

bool RawMasterTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

void RawMasterTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_MS_AsyncReceiveMessage, this, completionHandler);
}

void RawMasterTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_MS_CancelReceiveMessage, this);
}

RawSlaveTerminal::~RawSlaveTerminal()
{
    this->_destroy();
}

const std::string& RawSlaveTerminal::class_name() const
{
    static std::string s = "RawSlaveTerminal";
    return s;
}

terminal_type RawSlaveTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawSlaveTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_MS_Publish, this, data, size);
}

void RawSlaveTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawSlaveTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_MS_Publish, this, data, size);
}

bool RawSlaveTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

void RawSlaveTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_MS_AsyncReceiveMessage, this, completionHandler);
}

void RawSlaveTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_MS_CancelReceiveMessage, this);
}

RawCachedMasterTerminal::~RawCachedMasterTerminal()
{
    this->_destroy();
}

const std::string& RawCachedMasterTerminal::class_name() const
{
    static std::string s = "RawCachedMasterTerminal";
    return s;
}

terminal_type RawCachedMasterTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawCachedMasterTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_CMS_Publish, this, data, size);
}

void RawCachedMasterTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawCachedMasterTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_CMS_Publish, this, data, size);
}

bool RawCachedMasterTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

std::vector<char> RawCachedMasterTerminal::get_cached_message()
{
    return internal::get_cached_raw_message(YOGI_CMS_GetCachedMessage, this);
}

void RawCachedMasterTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_CMS_AsyncReceiveMessage, this, completionHandler);
}

void RawCachedMasterTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_CMS_CancelReceiveMessage, this);
}

RawCachedSlaveTerminal::~RawCachedSlaveTerminal()
{
    this->_destroy();
}

const std::string& RawCachedSlaveTerminal::class_name() const
{
    static std::string s = "RawCachedSlaveTerminal";
    return s;
}

terminal_type RawCachedSlaveTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawCachedSlaveTerminal::publish(const void* data, std::size_t size)
{
    internal::publish_raw_message(YOGI_CMS_Publish, this, data, size);
}

void RawCachedSlaveTerminal::publish(const std::vector<char>& data)
{
    publish(data.data(), data.size());
}

bool RawCachedSlaveTerminal::try_publish(const void* data, std::size_t size)
{
    return internal::try_publish_raw_message(YOGI_CMS_Publish, this, data, size);
}

bool RawCachedSlaveTerminal::try_publish(const std::vector<char>& data)
{
    return try_publish(data.data(), data.size());
}

std::vector<char> RawCachedSlaveTerminal::get_cached_message()
{
    return internal::get_cached_raw_message(YOGI_CMS_GetCachedMessage, this);
}

void RawCachedSlaveTerminal::async_receive_message(std::function<void (const Result&, std::vector<char>&&, cached_flag)> completionHandler)
{
    internal::async_receive_raw_message(YOGI_CMS_AsyncReceiveMessage, this, completionHandler);
}

void RawCachedSlaveTerminal::cancel_receive_message()
{
    internal::cancel(YOGI_CMS_CancelReceiveMessage, this);
}

RawServiceTerminal::Request::Request(RawServiceTerminal& terminal, ::yogi::raw_operation_id operationId, std::vector<char>&& data)
: m_terminal(&terminal)
, m_operationId(operationId)
, m_data(std::move(data))
{
}

RawServiceTerminal::Request::Request(Request&& other)
{
    *this = std::move(other);
}

RawServiceTerminal::Request& RawServiceTerminal::Request::operator= (Request&& other)
{
    m_terminal    = other.m_terminal;
    m_operationId = other.m_operationId;
    m_data        = std::move(other.m_data);

    other.m_terminal = nullptr;

    return *this;
}

void RawServiceTerminal::Request::respond(const void* data, std::size_t size)
{
    internal::respond_to_scattered_raw_message(YOGI_SC_RespondToRequest, m_terminal, m_operationId, data, size);
    m_operationId = 0;
}

bool RawServiceTerminal::Request::try_respond(const void* data, std::size_t size)
{
    int res = internal::try_respond_to_scattered_raw_message(YOGI_SC_RespondToRequest, m_terminal, m_operationId, data, size);
    if (res == YOGI_OK) {
        m_operationId = 0;
    }

    return res == YOGI_OK;
}

void RawServiceTerminal::Request::respond(const std::vector<char>& data)
{
    respond(data.data(), data.size());
}

bool RawServiceTerminal::Request::try_respond(const std::vector<char>& data)
{
    return try_respond(data.data(), data.size());
}

void RawServiceTerminal::Request::ignore()
{
    internal::ignore_scattered_message(YOGI_SC_IgnoreRequest, m_terminal, m_operationId);
    m_operationId = 0;
}

bool RawServiceTerminal::Request::try_ignore()
{
    int res = internal::try_ignore_scattered_message(YOGI_SC_IgnoreRequest, m_terminal, m_operationId);
    if (res == YOGI_OK) {
        m_operationId = 0;
    }

    return res == YOGI_OK;
}

RawServiceTerminal::~RawServiceTerminal()
{
    this->_destroy();
}

const std::string& RawServiceTerminal::class_name() const
{
    static std::string s = "RawServiceTerminal";
    return s;
}

terminal_type RawServiceTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

void RawServiceTerminal::async_receive_request(std::function<void (const Result&, Request&&)> completionHandler)
{
    internal::async_receive_scattered_raw_message(YOGI_SC_AsyncReceiveRequest, this, completionHandler);
}

void RawServiceTerminal::cancel_receive_request()
{
    internal::cancel(YOGI_SC_CancelReceiveRequest, this);
}

RawClientTerminal::Operation::Operation(RawClientTerminal& terminal, ::yogi::raw_operation_id operationId)
: m_terminal(&terminal)
, m_operationId(operationId)
{
}

RawClientTerminal::Operation::Operation()
: m_terminal(nullptr)
, m_operationId(0)
{
}

RawClientTerminal::Operation::Operation(Operation&& other)
{
    *this = std::move(other);
}

RawClientTerminal::Operation& RawClientTerminal::Operation::operator= (Operation&& other)
{
    m_terminal    = other.m_terminal;
    m_operationId = other.m_operationId;
    return *this;
}

void RawClientTerminal::Operation::cancel()
{
    internal::cancel_operation(YOGI_SC_CancelRequest, m_terminal, m_operationId);
    m_operationId = 0;
}

RawClientTerminal::Response::Response(RawClientTerminal& terminal, ::yogi::raw_operation_id operationId, gather_flags flags, std::vector<char>&& data)
: m_terminal(terminal)
, m_operationId(operationId)
, m_flags(flags)
, m_data(std::move(data))
{
}

RawClientTerminal::~RawClientTerminal()
{
    this->_destroy();
}

const std::string& RawClientTerminal::class_name() const
{
    static std::string s = "RawClientTerminal";
    return s;
}

terminal_type RawClientTerminal::type() const
{
    return static_cast<terminal_type>(TERMINAL_TYPE);
}

RawClientTerminal::Operation RawClientTerminal::async_request(const void* scatterData, std::size_t scatterSize,
    std::function<control_flow (const Result&, Response&&)> completionHandler)
{
    return internal::async_scatter_gather_raw<Operation>(YOGI_SC_AsyncRequest, this, scatterData, scatterSize, completionHandler);
}

RawClientTerminal::Operation RawClientTerminal::async_request(const std::vector<char>& data,
    std::function<control_flow (const Result&, Response&&)> completionHandler)
{
    return async_request(data.data(), data.size(), completionHandler);
}

RawClientTerminal::Operation RawClientTerminal::try_async_request(const void* scatterData, std::size_t scatterSize,
    std::function<control_flow (const Result&, Response&&)> completionHandler)
{
    try {
        return async_request(scatterData, scatterSize, completionHandler);
    }
    catch (...) {
        return Operation(*this);
    }
}

RawClientTerminal::Operation RawClientTerminal::try_async_request(const std::vector<char>& data,
    std::function<control_flow (const Result&, Response&&)> completionHandler)
{
    return try_async_request(data.data(), data.size(), completionHandler);
}

} // namespace yogi
