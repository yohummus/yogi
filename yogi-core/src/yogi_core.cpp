#include "config.h"
#include "scheduling/MultiThreadedScheduler.hpp"
#include "core/Node.hpp"
#include "core/Leaf.hpp"
#include "core/BindingT.hpp"
#include "core/deaf_mute/Terminal.hpp"
#include "core/publish_subscribe/Terminal.hpp"
#include "core/scatter_gather/Terminal.hpp"
#include "core/cached_publish_subscribe/Terminal.hpp"
#include "core/producer_consumer/Terminal.hpp"
#include "core/cached_producer_consumer/Terminal.hpp"
#include "core/master_slave/Terminal.hpp"
#include "core/cached_master_slave/Terminal.hpp"
#include "core/service_client/Terminal.hpp"
#include "connections/local/LocalConnection.hpp"
#include "connections/tcp/TcpServer.hpp"
#include "connections/tcp/TcpClient.hpp"
#include "api/PublicObjectRegister.hpp"
#include "api/TerminalWithBindingT.hpp"
#include "api/evaluate.hpp"
using namespace yogi;

#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <atomic>
#include <sstream>

#define CHECK_INITIALIZED()                                \
{{                                                         \
    if (!global::initialised) {                            \
        return YOGI_ERR_NOT_INITIALISED;                  \
    }                                                      \
}}

#define CHECK_HANDLE(handle)                               \
{{                                                         \
    if (!(handle)) {                                       \
        return YOGI_ERR_INVALID_HANDLE;                   \
    }                                                      \
}}

#define CHECK_PARAM(cond)                                  \
{{                                                         \
    if (!(cond)) {                                         \
        return YOGI_ERR_INVALID_PARAM;                    \
    }                                                      \
}}

#define CHECK_BUFFER(buffer, bufferSize)                   \
{{                                                         \
    CHECK_PARAM(buffer != nullptr || bufferSize == 0);     \
    CHECK_PARAM(bufferSize >= 0);                          \
}}

#define CHECK_ID(id)                                       \
{{                                                         \
    if (id <= 0) {                                         \
        return YOGI_ERR_INVALID_ID;                       \
    }                                                      \
}}

#define CHECK_FLAGS_THROW(flags, validFlags)               \
{{                                                         \
    if (flags & ~(validFlags)) {                           \
        throw api::ExceptionT<YOGI_ERR_INVALID_PARAM>{};  \
    }                                                      \
}}


namespace global {

static std::atomic<bool>                          initialised;
static boost::shared_ptr<boost::log::sinks::sink> logSink;

} // namespace global

namespace {

template <typename Param>
void param_to_stream(std::ostringstream& oss, Param param)
{
    oss << "0x" << param;
}

template <>
void param_to_stream<void**>(std::ostringstream& oss, void** param)
{
    oss << "0x" << param;

    if (param) {
        oss << " => 0x" << *param;
    }
}

template <>
void param_to_stream<int>(std::ostringstream& oss, int param)
{
    oss << param;
}

template <>
void param_to_stream<unsigned>(std::ostringstream& oss, unsigned param)
{
    oss << param;
}

template <>
void param_to_stream<const char*>(std::ostringstream& oss, const char* param)
{
    oss << "\"" << param << "\"";
}

template <typename... Params>
struct params_to_stream
{
    void operator() (std::ostringstream& oss, Params... params)
    {
    }
};

template <typename FirstParam, typename... RemainingParams>
struct params_to_stream<FirstParam, RemainingParams...>
{
    void operator() (std::ostringstream& oss, FirstParam firstParam,
        RemainingParams... remainingParams)
    {
        param_to_stream(oss, firstParam);
        oss << ", ";
        params_to_stream<RemainingParams...>()(oss, remainingParams...);
    }
};

template <typename LastParam>
struct params_to_stream<LastParam>
{
    void operator() (std::ostringstream& oss, LastParam lastParam)
    {
        param_to_stream(oss, lastParam);
    }
};

template <typename Fn, typename... Params>
int evaluate(Fn fn, const char* apiFnName, Params... params)
{
    int res = api::evaluate(fn);

    if (boost::log::core::get()->get_logging_enabled()) {
        std::ostringstream oss;
        oss << apiFnName << "(";
        params_to_stream<Params...>()(oss, params...);
        oss << "): " << res;
        if (res <= 0) {
            oss << " (" << api::Exception::get_description(res) << ")";
        }

        BOOST_LOG_TRIVIAL(debug) << oss.str();
    }

    return res;
}

int state_to_int(interfaces::IBinding::state_t state)
{
    switch (state) {
    case interfaces::IBinding::STATE_RELEASED:
        return YOGI_BD_RELEASED;

    case interfaces::IBinding::STATE_ESTABLISHED:
        return YOGI_BD_ESTABLISHED;

    default:
        YOGI_NEVER_REACHED;
        return -1;
    }
}

int state_to_int(interfaces::ISubscribableTerminal::state_t state)
{
    switch (state) {
    case interfaces::ISubscribableTerminal::STATE_UNSUBSCRIBED:
        return YOGI_BD_RELEASED;

    case interfaces::ISubscribableTerminal::STATE_SUBSCRIBED:
        return YOGI_BD_ESTABLISHED;

    default:
        YOGI_NEVER_REACHED;
        return -1;
    }
}

std::chrono::milliseconds int_to_timeout(int timeout)
{
	if (timeout == -1) {
		return std::chrono::milliseconds::max();
	}
	else {
		return std::chrono::milliseconds{timeout};
	}
}

} // anonymous namespace

YOGI_API const char* YOGI_GetVersion()
{
    return YOGI_VERSION;
}

YOGI_API const char* YOGI_GetErrorString(int errCode)
{
    return api::Exception::get_description(errCode);
}

YOGI_API int YOGI_SetLogFile(const char* file, int verbosity)
{
    return evaluate([&] {
        namespace logging  = boost::log;
        namespace keywords = boost::log::keywords;

        logging::trivial::severity_level severity_;
        switch (verbosity) {
        case YOGI_VB_FATAL:   severity_ = logging::trivial::fatal;   break;
        case YOGI_VB_ERROR:   severity_ = logging::trivial::error;   break;
        case YOGI_VB_WARNING: severity_ = logging::trivial::warning; break;
        case YOGI_VB_INFO:    severity_ = logging::trivial::info;    break;
        case YOGI_VB_DEBUG:   severity_ = logging::trivial::debug;   break;
        case YOGI_VB_TRACE:   severity_ = logging::trivial::trace;   break;
        default: throw api::ExceptionT<YOGI_ERR_INVALID_PARAM>{};
        }

        try {
            boost::log::core::get()->set_logging_enabled(true);

            boost::log::core::get()->remove_sink(global::logSink);

            global::logSink = logging::add_file_log
            (
                keywords::file_name  = file,
                keywords::format     = "[%TimeStamp%] [%ThreadID%]: %Message%",
                keywords::auto_flush = true
            );

            logging::add_common_attributes();

            logging::core::get()->set_filter
            (
                logging::trivial::severity >= severity_
            );

            BOOST_LOG_TRIVIAL(info) << "Log file created";
        }
        catch (...) {
            boost::log::core::get()->set_logging_enabled(false);
            throw api::ExceptionT<YOGI_ERR_CANNOT_CREATE_LOG_FILE>{};
        }
    }, __FUNCTION__, file, verbosity);
}

YOGI_API int YOGI_Initialise()
{
    if (global::initialised.exchange(true)) {
        return YOGI_ERR_ALREADY_INITIALISED;
    }

    if (!global::logSink) {
        boost::log::core::get()->set_logging_enabled(false);
    }

    return YOGI_OK;
}

YOGI_API int YOGI_Shutdown()
{
    if (!global::initialised.exchange(false)) {
        return YOGI_ERR_NOT_INITIALISED;
    }

    api::PublicObjectRegister::clear();

    return YOGI_OK;
}

YOGI_API int YOGI_Destroy(void* handle)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(handle);

    return evaluate([&] {
        api::PublicObjectRegister::destroy(handle);
    }, __FUNCTION__, handle);
}

YOGI_API int YOGI_CreateScheduler(void** scheduler)
{
    CHECK_INITIALIZED();
    CHECK_PARAM(scheduler);

    return evaluate([&] {
        *scheduler = api::PublicObjectRegister::create<
            scheduling::MultiThreadedScheduler>();
    }, __FUNCTION__, scheduler);
}

YOGI_API int YOGI_SetSchedulerThreadPoolSize(void* scheduler,
    unsigned numThreads)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(scheduler);
    CHECK_PARAM(numThreads > 0);

    return evaluate([&] {
        api::PublicObjectRegister::get_s<scheduling::MultiThreadedScheduler>(
            scheduler).resize_thread_pool(static_cast<std::size_t>(numThreads));
    }, __FUNCTION__, scheduler, numThreads);
}

YOGI_API int YOGI_CreateNode(void** node, void* scheduler)
{
    CHECK_INITIALIZED();
    CHECK_PARAM(node);
    CHECK_HANDLE(scheduler);

    return evaluate([&] {
        auto& scheduler_ = api::PublicObjectRegister::get_s<
            interfaces::IScheduler>(scheduler);

        *node = api::PublicObjectRegister::create<core::Node>(scheduler_);
    }, __FUNCTION__, node, scheduler);
}

YOGI_API int YOGI_GetKnownTerminals(void* node, void* buffer,
    unsigned bufferSize, unsigned* numTerminals)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(node);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(numTerminals);

    return evaluate([&] {
        auto& node_ = api::PublicObjectRegister::get_s<core::Node>(node);
        auto buffer_ = static_cast<char*>(buffer);
        auto bufferSize_ = static_cast<std::size_t>(bufferSize);

        std::vector<char> tmp;
        std::size_t bufferOffset = 0;
        *numTerminals = 0;

        auto convertFn = [&](char type, std::vector<base::Identifier>& tms) {
            for (auto& identifier : tms) {
                tmp.resize(1 + 4 + identifier.name().size() + 1);
                if (tmp.size() > bufferSize_ - bufferOffset) {
                    throw api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>{};
                }

                tmp[0] = type;

                std::uint32_t signature = static_cast<std::uint32_t>(
                    identifier.signature());
                std::copy_n(reinterpret_cast<char*>(&signature), 4, tmp.begin()
                    + 1);

                std::copy(identifier.name().begin(), identifier.name().end(),
                    tmp.begin() + 5);
                tmp.back() = '\0';

                std::copy(tmp.begin(), tmp.end(), buffer_ + bufferOffset);
                bufferOffset += tmp.size();

                ++*numTerminals;
            }
        };

        auto v = node_.get_known_terminals();
        convertFn(YOGI_TM_DEAFMUTE,               v.deafMute);
        convertFn(YOGI_TM_PUBLISHSUBSCRIBE,       v.publishSubscribe);
        convertFn(YOGI_TM_SCATTERGATHER,          v.scatterGather);
        convertFn(YOGI_TM_CACHEDPUBLISHSUBSCRIBE, v.cachedPublishSubscribe);
        convertFn(YOGI_TM_PRODUCER,               v.producerConsumer);
        convertFn(YOGI_TM_CACHEDPRODUCER,         v.cachedProducerConsumer);
        convertFn(YOGI_TM_MASTER,                 v.masterSlave);
        convertFn(YOGI_TM_CACHEDMASTER,           v.cachedMasterSlave);
        convertFn(YOGI_TM_SERVICE,                v.serviceClient);
    }, __FUNCTION__, node, buffer, bufferSize, numTerminals);
}

YOGI_API int YOGI_AsyncAwaitKnownTerminalsChange(void* node, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(node);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& node_ = api::PublicObjectRegister::get_s<core::Node>(node);
        auto buffer_ = static_cast<char*>(buffer);
        auto bufferSize_ = static_cast<std::size_t>(bufferSize);

        node_.async_await_known_terminals_change(
            [=](const api::Exception& e,
                core::Node::known_terminal_change_info info) {
                    if (bufferSize_ < info.identifier.name().size() + 7) {
                        handlerFn(YOGI_ERR_BUFFER_TOO_SMALL, userArg);
                    }
                    else {
                        buffer_[0] = (info.added ? 1 : 0);
                        buffer_[1] = static_cast<char>(info.type);
                        std::copy_n(reinterpret_cast<const char*>(
                            &info.identifier.signature()), 4, buffer_ + 2);
                        std::copy(info.identifier.name().begin(),
                            info.identifier.name().end(), buffer_ + 6);
                        buffer_[info.identifier.name().size() + 6] = '\0';
                        handlerFn(e.error_code(), userArg);
                    }
            }
        );
    }, __FUNCTION__, node, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_CancelAwaitKnownTerminalsChange(void* node)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(node);

    return evaluate([&] {
        auto& node_ = api::PublicObjectRegister::get_s<core::Node>(node);

        node_.cancel_await_known_terminals_change();
    }, __FUNCTION__, node);
}

YOGI_API int YOGI_CreateLeaf(void** leaf, void* scheduler)
{
    CHECK_INITIALIZED();
    CHECK_PARAM(leaf);
    CHECK_HANDLE(scheduler);

    return evaluate([&] {
        auto& scheduler_ = api::PublicObjectRegister::get_s<
            interfaces::IScheduler>(scheduler);

        *leaf = api::PublicObjectRegister::create<core::Leaf>(scheduler_);
    }, __FUNCTION__, leaf, scheduler);
}

YOGI_API int YOGI_CreateTerminal(void** terminal, void* leaf, int type,
    const char* name, unsigned signature)
{
    CHECK_INITIALIZED();
    CHECK_PARAM(terminal);
    CHECK_HANDLE(leaf);
    CHECK_PARAM(name);
    CHECK_PARAM(*name != '\0');

    return evaluate([&] {
        auto& leaf_ = api::PublicObjectRegister::get_s<core::Leaf>(leaf);
        auto signature_ = static_cast<base::Identifier::signature_type>(
            signature);

        switch (type) {
        case YOGI_TM_DEAFMUTE:
            *terminal = api::PublicObjectRegister::create<
                core::deaf_mute::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, false});
            break;

        case YOGI_TM_PUBLISHSUBSCRIBE:
            *terminal = api::PublicObjectRegister::create<
                core::publish_subscribe::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, false});
            break;

        case YOGI_TM_SCATTERGATHER:
            *terminal = api::PublicObjectRegister::create<
                core::scatter_gather::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, false});
            break;

        case YOGI_TM_CACHEDPUBLISHSUBSCRIBE:
            *terminal = api::PublicObjectRegister::create<
                core::cached_publish_subscribe::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, false});
            break;

        case YOGI_TM_PRODUCER:
            *terminal = api::PublicObjectRegister::create<
                core::producer_consumer::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, false});
            break;

        case YOGI_TM_CONSUMER: {{
            auto tm = std::make_shared<api::TerminalWithBindingT<
                core::producer_consumer::logic_types<>>>(leaf_,
                    signature_, name, true);
            tm->create_binding();

            *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_CACHEDPRODUCER:
            *terminal = api::PublicObjectRegister::create<
                core::cached_producer_consumer::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, false});
            break;

        case YOGI_TM_CACHEDCONSUMER: {{
            auto tm = std::make_shared<api::TerminalWithBindingT<
                core::cached_producer_consumer::logic_types<>>>(leaf_,
                    signature_, name, true);
            tm->create_binding();

            *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_MASTER: {{
                auto tm = std::make_shared<api::TerminalWithBindingT<
                    core::master_slave::logic_types<>>>(leaf_,
                        signature_, name, false);
                tm->create_binding();

                *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_SLAVE: {{
                auto tm = std::make_shared<api::TerminalWithBindingT<
                    core::master_slave::logic_types<>>>(leaf_,
                        signature_, name, true);
                tm->create_binding();

                *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_CACHEDMASTER: {{
                auto tm = std::make_shared<api::TerminalWithBindingT<
                    core::cached_master_slave::logic_types<>>>(leaf_,
                        signature_, name, false);
                tm->create_binding();

                *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_CACHEDSLAVE: {{
                auto tm = std::make_shared<api::TerminalWithBindingT<
                    core::cached_master_slave::logic_types<>>>(leaf_,
                        signature_, name, true);
                tm->create_binding();

                *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_SERVICE: {{
            auto tm = std::make_shared<api::TerminalWithBindingT<
                core::service_client::logic_types<>>>(leaf_,
                    signature_, name, false);
            tm->create_binding();

            *terminal = api::PublicObjectRegister::add(tm);
            }} break;

        case YOGI_TM_CLIENT:
            *terminal = api::PublicObjectRegister::create<
                core::service_client::Terminal<>>(leaf_,
                    base::Identifier{signature_, name, true});
            break;

        default:
            throw api::ExceptionT<YOGI_ERR_INVALID_PARAM>{};
        }
    }, __FUNCTION__, terminal, leaf, type, name, signature);
}

YOGI_API int YOGI_GetSubscriptionState(void* terminal, int* state)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_PARAM(state);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            interfaces::ISubscribableTerminal>(terminal);

        *state = state_to_int(terminal_.subscription_state());
    }, __FUNCTION__, terminal, state);
}

YOGI_API int YOGI_AsyncGetSubscriptionState(void* terminal,
    void (*handlerFn)(int, int, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            interfaces::ISubscribableTerminal>(terminal);

        terminal_.async_get_subscription_state([=](const api::Exception& e,
            interfaces::ISubscribableTerminal::state_t state) {
                handlerFn(e.error_code(), state_to_int(state), userArg);
        });
    }, __FUNCTION__, terminal, handlerFn, userArg);
}

YOGI_API int YOGI_AsyncAwaitSubscriptionStateChange(void* terminal,
    void (*handlerFn)(int, int, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            interfaces::ISubscribableTerminal>(terminal);

        terminal_.async_await_subscription_state_change([=](
            const api::Exception& e,
            interfaces::ISubscribableTerminal::state_t state) {
                handlerFn(e.error_code(), state_to_int(state), userArg);
        });
    }, __FUNCTION__, terminal, handlerFn, userArg);
}

YOGI_API int YOGI_CancelAwaitSubscriptionStateChange(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            interfaces::ISubscribableTerminal>(terminal);

        terminal_.cancel_await_subscription_state_change();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_CreateBinding(void** binding, void* terminal,
    const char* targets)
{
    CHECK_INITIALIZED();
    CHECK_PARAM(binding);
    CHECK_HANDLE(terminal);
    CHECK_PARAM(targets);
    CHECK_PARAM(*targets != '\0');

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<interfaces::ITerminal>(
            terminal);

        if (dynamic_cast<core::deaf_mute::Terminal<>*>(&terminal_)) {
            typedef core::deaf_mute::Terminal<>                  tm_type;
            typedef core::BindingT<core::deaf_mute::LeafLogic<>> bd_type;

            *binding = api::PublicObjectRegister::create<bd_type>(
                static_cast<tm_type&>(terminal_), targets, false);
        }
        else if (dynamic_cast<core::publish_subscribe::Terminal<>*>(&terminal_)) {
            typedef core::publish_subscribe::Terminal<>                  tm_type;
            typedef core::BindingT<core::publish_subscribe::LeafLogic<>> bd_type;

            *binding = api::PublicObjectRegister::create<bd_type>(
                static_cast<tm_type&>(terminal_), targets, false);
        }
        else if (dynamic_cast<core::scatter_gather::Terminal<>*>(&terminal_)) {
            typedef core::scatter_gather::Terminal<>                  tm_type;
            typedef core::BindingT<core::scatter_gather::LeafLogic<>> bd_type;

            *binding = api::PublicObjectRegister::create<bd_type>(
                static_cast<tm_type&>(terminal_), targets, false);
        }
        else if (dynamic_cast<core::cached_publish_subscribe::Terminal<>*>(&terminal_)) {
            typedef core::cached_publish_subscribe::Terminal<>                  tm_type;
            typedef core::BindingT<core::cached_publish_subscribe::LeafLogic<>> bd_type;

            *binding = api::PublicObjectRegister::create<bd_type>(
                static_cast<tm_type&>(terminal_), targets, false);
        }
        else {
            throw api::ExceptionT<YOGI_ERR_INVALID_HANDLE>{};
        }
    }, __FUNCTION__, binding, terminal, targets);
}

YOGI_API int YOGI_GetBindingState(void* object, int* state)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(object);
    CHECK_PARAM(state);

    return evaluate([&] {
        auto& binding_ = api::PublicObjectRegister::get_s<interfaces::IBinding>(
            object);

        *state = state_to_int(binding_.state());
    }, __FUNCTION__, object, state);
}

YOGI_API int YOGI_AsyncGetBindingState(void* binding,
    void (*handlerFn)(int, int, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(binding);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& binding_ = api::PublicObjectRegister::get_s<interfaces::IBinding>(
            binding);

        binding_.async_get_state([=](const api::Exception& e,
            interfaces::IBinding::state_t state) {
                handlerFn(e.error_code(), state_to_int(state), userArg);
        });
    }, __FUNCTION__, binding, handlerFn, userArg);
}

YOGI_API int YOGI_AsyncAwaitBindingStateChange(void* binding,
    void (*handlerFn)(int, int, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(binding);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& binding_ = api::PublicObjectRegister::get_s<interfaces::IBinding>(
            binding);

        binding_.async_await_state_change([=](const api::Exception& e,
            interfaces::IBinding::state_t state) {
                handlerFn(e.error_code(), state_to_int(state), userArg);
        });
    }, __FUNCTION__, binding, handlerFn, userArg);
}

YOGI_API int YOGI_CancelAwaitBindingStateChange(void* binding)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(binding);

    return evaluate([&] {
        auto& binding_ = api::PublicObjectRegister::get_s<interfaces::IBinding>(
            binding);

        binding_.cancel_await_state_change();
    }, __FUNCTION__, binding);
}

YOGI_API int YOGI_CreateLocalConnection(void** connection, void* leafNodeA,
    void* leafNodeB)
{
    CHECK_INITIALIZED();
    CHECK_PARAM(connection);
    CHECK_HANDLE(leafNodeA);
    CHECK_HANDLE(leafNodeB);

    return evaluate([&] {
        auto& communicatorA = api::PublicObjectRegister::get_s<
            interfaces::ICommunicator>(leafNodeA);
        auto& communicatorB = api::PublicObjectRegister::get_s<
            interfaces::ICommunicator>(leafNodeB);

        *connection = api::PublicObjectRegister::create<
			connections::local::LocalConnection>(communicatorA, communicatorB);
    }, __FUNCTION__, connection, leafNodeA, leafNodeB);
}

YOGI_API int YOGI_CreateTcpServer(void** tcpServer, void* scheduler,
	const char* address, unsigned port, const void* ident, unsigned identSize)
{
	CHECK_INITIALIZED();
	CHECK_PARAM(tcpServer);
	CHECK_HANDLE(scheduler);
	CHECK_PARAM(address);
	CHECK_PARAM(0 < port && port < 65536);
	CHECK_PARAM(ident != nullptr || identSize == 0);

	return evaluate([&] {
		auto& scheduler_ = api::PublicObjectRegister::get_s<
			interfaces::IScheduler>(scheduler);

		*tcpServer = api::PublicObjectRegister::create<
			connections::tcp::TcpServer>(scheduler_, std::string{address},
				static_cast<unsigned short>(port),
				boost::asio::buffer(ident, identSize));
	}, __FUNCTION__, tcpServer, scheduler, address, port, ident, identSize);
}

YOGI_API int YOGI_AsyncTcpAccept(void* tcpServer, int hsTimeout,
	void (*handlerFn)(int, void*, void*), void* userArg)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(tcpServer);
	CHECK_PARAM(hsTimeout == -1 || hsTimeout > 0);
	CHECK_PARAM(handlerFn);

	return evaluate([&] {
		auto& server_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpServer>(tcpServer);

		server_.async_accept([=](const api::Exception& e,
			connections::tcp::tcp_connection_ptr conn) {
            void* connection = conn.get();
				if (conn) {
					api::PublicObjectRegister::add(conn);
               conn.reset();
				}
				handlerFn(e.error_code(), connection, userArg);
		}, int_to_timeout(hsTimeout));
	}, __FUNCTION__, tcpServer, hsTimeout, handlerFn, userArg);
}

YOGI_API int YOGI_CancelTcpAccept(void* tcpServer)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(tcpServer);

	return evaluate([&] {
		auto& server_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpServer>(tcpServer);

		server_.cancel_accept();
	}, __FUNCTION__, tcpServer);
}

YOGI_API int YOGI_CreateTcpClient(void** tcpClient, void* scheduler,
	const void* ident, unsigned identSize)
{
	CHECK_INITIALIZED();
	CHECK_PARAM(tcpClient);
	CHECK_HANDLE(scheduler);
	CHECK_PARAM(ident != nullptr || identSize == 0);

	return evaluate([&] {
		auto& scheduler_ = api::PublicObjectRegister::get_s<
			interfaces::IScheduler>(scheduler);

		*tcpClient = api::PublicObjectRegister::create<
			connections::tcp::TcpClient>(scheduler_,
				boost::asio::buffer(ident, identSize));
	}, __FUNCTION__, tcpClient, scheduler, ident, identSize);
}

YOGI_API int YOGI_AsyncTcpConnect(void* tcpClient, const char* host,
    unsigned port, int hsTimeout, void (*handlerFn)(int, void*, void*),
    void* userArg)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(tcpClient);
	CHECK_PARAM(host);
	CHECK_PARAM(0 < port && port < 65536);
	CHECK_PARAM(hsTimeout == -1 || hsTimeout > 0);
	CHECK_PARAM(handlerFn);

	return evaluate([&] {
		auto& client_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpClient>(tcpClient);

		client_.async_connect(host, static_cast<unsigned short>(port),
			[=](const api::Exception& e,
			connections::tcp::tcp_connection_ptr conn) {
            void* connection = conn.get();
				if (conn) {
					api::PublicObjectRegister::add(conn);
               conn.reset();
				}
				handlerFn(e.error_code(), connection, userArg);
		}, int_to_timeout(hsTimeout));
	}, __FUNCTION__, tcpClient, host, port, hsTimeout, handlerFn, userArg);
}

YOGI_API int YOGI_CancelTcpConnect(void* tcpClient)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(tcpClient);

	return evaluate([&] {
		auto& client_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpClient>(tcpClient);

		client_.cancel_connect();
	}, __FUNCTION__, tcpClient);
}

YOGI_API int YOGI_GetConnectionDescription(void* connection, char* buffer,
    unsigned bufferSize)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(connection);
	CHECK_BUFFER(buffer, bufferSize);

	return evaluate([&] {
		auto& connection_ = api::PublicObjectRegister::get_s<
			interfaces::IConnectionLike>(connection);
		auto bufferSize_ = static_cast<std::size_t>(bufferSize);

		auto& s = connection_.description();
		if (bufferSize_ > s.size()) {
			std::copy(s.begin(), s.end(), buffer);
			buffer[s.size()] = '\0';
		}
		else {
			std::copy_n(s.begin(), bufferSize_, buffer);
			throw api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>{};
		}
	}, __FUNCTION__, connection, buffer, bufferSize);
}

YOGI_API int YOGI_GetRemoteVersion(void* connection, char* buffer,
    unsigned bufferSize)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(connection);
    CHECK_BUFFER(buffer, bufferSize);

	return evaluate([&] {
		auto& connection_ = api::PublicObjectRegister::get_s<
            interfaces::IConnectionLike>(connection);
		auto buffer_ = reinterpret_cast<char*>(buffer);
		auto bufferSize_ = static_cast<std::size_t>(bufferSize);

		auto& s = connection_.remote_version();
		if (bufferSize_ > s.size()) {
			std::copy(s.begin(), s.end(), buffer_);
			buffer[s.size()] = '\0';
		}
		else {
			std::copy_n(s.begin(), bufferSize_, buffer_);
			throw api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>{};
		}
	}, __FUNCTION__, connection, buffer, bufferSize);
}

YOGI_API int YOGI_GetRemoteIdentification(void* connection, void* buffer,
    unsigned bufferSize, unsigned* size)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(connection);
    CHECK_BUFFER(buffer, bufferSize);

	return evaluate([&] {
		auto& connection_ = api::PublicObjectRegister::get_s<
            interfaces::IConnectionLike>(connection);
		auto buffer_ = reinterpret_cast<char*>(buffer);
		auto bufferSize_ = static_cast<std::size_t>(bufferSize);

		auto& data = connection_.remote_identification();

		if (size) {
			*size = static_cast<unsigned>(data.size());
		}

		if (bufferSize_ > data.size()) {
			std::copy(data.begin(), data.end(), buffer_);
		}
		else {
			std::copy_n(data.begin(), bufferSize_, buffer_);
			throw api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>{};
		}
	}, __FUNCTION__, connection, buffer, bufferSize, size);
}

YOGI_API int YOGI_AssignConnection(void* connection, void* leafNode,
    int timeout)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(connection);
	CHECK_HANDLE(leafNode);
	CHECK_PARAM(timeout == -1 || timeout > 0);

	return evaluate([&] {
		auto& connection_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpConnection>(connection);
		auto& communicator_ = api::PublicObjectRegister::get_s<
			interfaces::ICommunicator>(leafNode);

		connection_.assign(communicator_, int_to_timeout(timeout));
	}, __FUNCTION__, connection, leafNode, timeout);
}

YOGI_API int YOGI_AsyncAwaitConnectionDeath(void* connection,
	void (*handlerFn)(int, void*), void* userArg)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(connection);
	CHECK_PARAM(handlerFn);

	return evaluate([&] {
		auto& connection_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpConnection>(connection);

		connection_.async_await_death([=](const api::Exception& e) {
			handlerFn(e.error_code(), userArg);
		});
	}, __FUNCTION__, connection, handlerFn, userArg);
}

YOGI_API int YOGI_CancelAwaitConnectionDeath(void* connection)
{
	CHECK_INITIALIZED();
	CHECK_HANDLE(connection);

	return evaluate([&] {
		auto& connection_ = api::PublicObjectRegister::get_s<
			connections::tcp::TcpConnection>(connection);

		connection_.cancel_await_death();
	}, __FUNCTION__, connection);
}

YOGI_API int YOGI_PS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::publish_subscribe::Terminal<>>(terminal);

        bool ok = terminal_.publish(base::Buffer{buffer,
            static_cast<std::size_t>(bufferSize)});

        return ok ? YOGI_OK : YOGI_ERR_NOT_BOUND;
    }, __FUNCTION__, terminal, buffer, bufferSize);
}

YOGI_API int YOGI_PS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::publish_subscribe::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        terminal_.async_receive_published_message(buffer_, [=](
            const api::Exception& e, std::size_t size) {
                handlerFn(e.error_code(), static_cast<unsigned>(size), userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_PS_CancelReceiveMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::publish_subscribe::Terminal<>>(terminal);

        terminal_.cancel_receive_published_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_SG_AsyncScatterGather(void* terminal, const void* scatBuf,
    unsigned scatSize, void* gathBuf, unsigned gathSize,
    int (*handlerFn)(int, int, int, unsigned, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_PARAM(scatBuf != nullptr || scatSize == 0);
    CHECK_PARAM(gathBuf != nullptr || gathSize == 0);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::scatter_gather::Terminal<>>(terminal);
        auto gathBuf_ = boost::asio::buffer(gathBuf, static_cast<std::size_t>(
            gathSize));

        base::Id id = terminal_.async_scatter_gather(base::Buffer{scatBuf,
            static_cast<std::size_t>(scatSize)}, gathBuf_, [=](
            const api::Exception& e, base::Id operationId,
            core::scatter_gather::gather_flags flags, std::size_t size) {
                return !handlerFn(e.error_code(),
                    static_cast<int>(operationId.number()),
                    static_cast<int>(flags),
                    static_cast<unsigned>(size), userArg);
        });

        return static_cast<int>(id.number());
    }, __FUNCTION__, terminal, scatBuf, scatSize, gathBuf, gathSize, handlerFn,
        userArg);
}

YOGI_API int YOGI_SG_CancelScatterGather(void* terminal, int operationId)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_ID(operationId);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::scatter_gather::Terminal<>>(terminal);

        terminal_.cancel_scatter_gather(base::Id{
            static_cast<base::Id::number_type>(operationId)});
    }, __FUNCTION__, terminal, operationId);
}

YOGI_API int YOGI_SG_AsyncReceiveScatteredMessage(void* terminal,
    void* buffer, unsigned bufferSize,
    void (*handlerFn)(int, int, unsigned, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::scatter_gather::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        terminal_.async_receive_scattered_message(buffer_, [=](
            const api::Exception& e, base::Id operationId,
            std::size_t size) {
                handlerFn(e.error_code(),
                    static_cast<int>(operationId.number()),
                    static_cast<unsigned>(size), userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_SG_CancelReceiveScatteredMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::scatter_gather::Terminal<>>(terminal);

        terminal_.cancel_receive_scattered_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_SG_RespondToScatteredMessage(void* terminal,
    int operationId, const void* buffer, unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_ID(operationId);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::scatter_gather::Terminal<>>(terminal);

        terminal_.respond_to_scattered_message(
            base::Id{static_cast<base::Id::number_type>(operationId)},
            base::Buffer{buffer, static_cast<std::size_t>(bufferSize)});
    }, __FUNCTION__, terminal, operationId, buffer, bufferSize);
}

YOGI_API int YOGI_SG_IgnoreScatteredMessage(void* terminal, int operationId)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_ID(operationId);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::scatter_gather::Terminal<>>(terminal);

        terminal_.ignore_scattered_message(
            base::Id{static_cast<base::Id::number_type>(operationId)});
    }, __FUNCTION__, terminal, operationId);
}

YOGI_API int YOGI_CPS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_publish_subscribe::Terminal<>>(terminal);

        bool ok = terminal_.publish(base::Buffer{buffer,
            static_cast<std::size_t>(bufferSize)});

        return ok ? YOGI_OK : YOGI_ERR_NOT_BOUND;
    }, __FUNCTION__, terminal, buffer, bufferSize);
}

YOGI_API int YOGI_CPS_GetCachedMessage(void* terminal, void* buffer,
    unsigned bufferSize, unsigned* bytesWritten)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(bytesWritten);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_publish_subscribe::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        std::pair<bool, std::size_t> res;
        try {
            res = terminal_.get_cache(buffer_);
            *bytesWritten = static_cast<unsigned>(res.second);
        }
        catch (const api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>&) {
            *bytesWritten = bufferSize;
            throw;
        }

        return res.first ? YOGI_OK : YOGI_ERR_UNINITIALIZED;
    }, __FUNCTION__, terminal, buffer, bufferSize, bytesWritten);
}

YOGI_API int YOGI_CPS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, int, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_publish_subscribe::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        terminal_.async_receive_published_message(buffer_, [=](
            const api::Exception& e, std::size_t size, bool cached) {
            handlerFn(e.error_code(), static_cast<unsigned>(size),
                cached ? 1 : 0, userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_CPS_CancelReceiveMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_publish_subscribe::Terminal<>>(terminal);

        terminal_.cancel_receive_published_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_PC_Publish(void* terminal, const void* buffer,
    unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::producer_consumer::Terminal<>>(terminal);

        // allow only Producer Terminals
        if (terminal_.identifier().hidden()) {
            return YOGI_ERR_WRONG_OBJECT_TYPE;
        }

        bool ok = terminal_.publish(base::Buffer{buffer,
            static_cast<std::size_t>(bufferSize)});

        return ok ? YOGI_OK : YOGI_ERR_NOT_BOUND;
    }, __FUNCTION__, terminal, buffer, bufferSize);
}

YOGI_API int YOGI_PC_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::producer_consumer::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        // allow only Consumer Terminals
        if (!terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.async_receive_published_message(buffer_, [=](
            const api::Exception& e, std::size_t size) {
            handlerFn(e.error_code(), static_cast<unsigned>(size), userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_PC_CancelReceiveMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::producer_consumer::Terminal<>>(terminal);

        // allow only Consumer Terminals
        if (!terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.cancel_receive_published_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_CPC_Publish(void* terminal, const void* buffer,
    unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_producer_consumer::Terminal<>>(terminal);

        // allow only Cached Producer Terminals
        if (terminal_.identifier().hidden()) {
            return YOGI_ERR_WRONG_OBJECT_TYPE;
        }

        bool ok = terminal_.publish(base::Buffer{buffer,
            static_cast<std::size_t>(bufferSize)});

        return ok ? YOGI_OK : YOGI_ERR_NOT_BOUND;
    }, __FUNCTION__, terminal, buffer, bufferSize);
}

YOGI_API int YOGI_CPC_GetCachedMessage(void* terminal, void* buffer,
    unsigned bufferSize, unsigned* bytesWritten)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(bytesWritten);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_producer_consumer::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        // allow only Cached Consumer Terminals
        if (!terminal_.identifier().hidden()) {
            return YOGI_ERR_WRONG_OBJECT_TYPE;
        }

        std::pair<bool, std::size_t> res;
        try {
            res = terminal_.get_cache(buffer_);
            *bytesWritten = static_cast<unsigned>(res.second);
        }
        catch (const api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>&) {
            *bytesWritten = bufferSize;
            throw;
        }

        return res.first ? YOGI_OK : YOGI_ERR_UNINITIALIZED;
    }, __FUNCTION__, terminal, buffer, bufferSize, bytesWritten);
}

YOGI_API int YOGI_CPC_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, int, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_producer_consumer::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        // allow only Cached Consumer Terminals
        if (!terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.async_receive_published_message(buffer_, [=](
            const api::Exception& e, std::size_t size, bool cached) {
            handlerFn(e.error_code(), static_cast<unsigned>(size),
                cached ? 1 : 0, userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_CPC_CancelReceiveMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_producer_consumer::Terminal<>>(terminal);

        // allow only Cached Consumer Terminals
        if (!terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.cancel_receive_published_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_MS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::master_slave::Terminal<>>(terminal);

        bool ok = terminal_.publish(base::Buffer{buffer,
            static_cast<std::size_t>(bufferSize)});

        return ok ? YOGI_OK : YOGI_ERR_NOT_BOUND;
    }, __FUNCTION__, terminal, buffer, bufferSize);
}

YOGI_API int YOGI_MS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::master_slave::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        terminal_.async_receive_published_message(buffer_, [=](
            const api::Exception& e, std::size_t size) {
            handlerFn(e.error_code(), static_cast<unsigned>(size), userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_MS_CancelReceiveMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::master_slave::Terminal<>>(terminal);

        terminal_.cancel_receive_published_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_CMS_Publish(void* terminal, const void* buffer,
    unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_master_slave::Terminal<>>(terminal);

        bool ok = terminal_.publish(base::Buffer{buffer,
            static_cast<std::size_t>(bufferSize)});

        return ok ? YOGI_OK : YOGI_ERR_NOT_BOUND;
    }, __FUNCTION__, terminal, buffer, bufferSize);
}

YOGI_API int YOGI_CMS_GetCachedMessage(void* terminal, void* buffer,
    unsigned bufferSize, unsigned* bytesWritten)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(bytesWritten);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_master_slave::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        std::pair<bool, std::size_t> res;
        try {
            res = terminal_.get_cache(buffer_);
            *bytesWritten = static_cast<unsigned>(res.second);
        }
        catch (const api::ExceptionT<YOGI_ERR_BUFFER_TOO_SMALL>&) {
            *bytesWritten = bufferSize;
            throw;
        }

        return res.first ? YOGI_OK : YOGI_ERR_UNINITIALIZED;
    }, __FUNCTION__, terminal, buffer, bufferSize, bytesWritten);
}

YOGI_API int YOGI_CMS_AsyncReceiveMessage(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, unsigned, int, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_master_slave::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        terminal_.async_receive_published_message(buffer_, [=](
            const api::Exception& e, std::size_t size, bool cached) {
            handlerFn(e.error_code(), static_cast<unsigned>(size),
                cached ? 1 : 0, userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_CMS_CancelReceiveMessage(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::cached_master_slave::Terminal<>>(terminal);

        terminal_.cancel_receive_published_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_SC_AsyncRequest(void* terminal, const void* reqBuf,
    unsigned reqSize, void* respBuf, unsigned respSize,
    int (*handlerFn)(int, int, int, unsigned, void*), void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_PARAM(reqBuf != nullptr || reqSize == 0);
    CHECK_PARAM(respBuf != nullptr || respSize == 0);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::service_client::Terminal<>>(terminal);
        auto respBuf_ = boost::asio::buffer(respBuf, static_cast<std::size_t>(
            respSize));

        // allow only Client Terminals
        if (!terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        base::Id id = terminal_.async_scatter_gather(base::Buffer{reqBuf,
            static_cast<std::size_t>(reqSize)}, respBuf_, [=](
                const api::Exception& e, base::Id operationId,
                core::scatter_gather::gather_flags flags, std::size_t size) {
            return !handlerFn(e.error_code(),
                static_cast<int>(operationId.number()),
                static_cast<int>(flags),
                static_cast<unsigned>(size), userArg);
        });

        return static_cast<int>(id.number());
    }, __FUNCTION__, terminal, reqBuf, reqSize, respBuf, respSize, handlerFn,
        userArg);
}

YOGI_API int YOGI_SC_CancelRequest(void* terminal, int operationId)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_ID(operationId);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::service_client::Terminal<>>(terminal);

        // allow only Client Terminals
        if (!terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.cancel_scatter_gather(base::Id{
            static_cast<base::Id::number_type>(operationId)});
    }, __FUNCTION__, terminal, operationId);
}

YOGI_API int YOGI_SC_AsyncReceiveRequest(void* terminal, void* buffer,
    unsigned bufferSize, void (*handlerFn)(int, int, unsigned, void*),
    void* userArg)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_BUFFER(buffer, bufferSize);
    CHECK_PARAM(handlerFn);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::service_client::Terminal<>>(terminal);
        auto buffer_ = boost::asio::buffer(buffer, static_cast<std::size_t>(
            bufferSize));

        // allow only Service Terminals
        if (terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.async_receive_scattered_message(buffer_, [=](
            const api::Exception& e, base::Id operationId,
            std::size_t size) {
            handlerFn(e.error_code(),
                static_cast<int>(operationId.number()),
                static_cast<unsigned>(size), userArg);
        });
    }, __FUNCTION__, terminal, buffer, bufferSize, handlerFn, userArg);
}

YOGI_API int YOGI_SC_CancelReceiveRequest(void* terminal)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::service_client::Terminal<>>(terminal);

        // allow only Service Terminals
        if (terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.cancel_receive_scattered_message();
    }, __FUNCTION__, terminal);
}

YOGI_API int YOGI_SC_RespondToRequest(void* terminal,
    int operationId, const void* buffer, unsigned bufferSize)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_ID(operationId);
    CHECK_BUFFER(buffer, bufferSize);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::service_client::Terminal<>>(terminal);

        // allow only Service Terminals
        if (terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.respond_to_scattered_message(
            base::Id{static_cast<base::Id::number_type>(operationId)},
            base::Buffer{buffer, static_cast<std::size_t>(bufferSize)});
    }, __FUNCTION__, terminal, operationId, buffer, bufferSize);
}

YOGI_API int YOGI_SC_IgnoreRequest(void* terminal, int operationId)
{
    CHECK_INITIALIZED();
    CHECK_HANDLE(terminal);
    CHECK_ID(operationId);

    return evaluate([&] {
        auto& terminal_ = api::PublicObjectRegister::get_s<
            core::service_client::Terminal<>>(terminal);

        // allow only Service Terminals
        if (terminal_.identifier().hidden()) {
            throw api::ExceptionT<YOGI_ERR_WRONG_OBJECT_TYPE>{};
        }

        terminal_.ignore_scattered_message(
            base::Id{static_cast<base::Id::number_type>(operationId)});
    }, __FUNCTION__, terminal, operationId);
}
