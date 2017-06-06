#include "TestService.hpp"
#include "helpers.hpp"
#include "proto/yogi_00000000.h"
#include "proto/yogi_00000001.h"
#include "proto/yogi_00000002.h"
#include "proto/yogi_00000003.h"
#include "proto/yogi_00000004.h"
#include "proto/yogi_00000005.h"
#include "proto/yogi_00000006.h"
#include "proto/yogi_00000007.h"
#include "proto/yogi_00000008.h"
#include "proto/yogi_00000009.h"
#include "proto/yogi_0000000a.h"
#include "proto/yogi_0000000b.h"
#include "proto/yogi_0000000c.h"
#include "proto/yogi_0000000d.h"
#include "proto/yogi_0000000e.h"
#include "proto/yogi_0000000f.h"
#include "proto/yogi_00000010.h"
#include "proto/yogi_000000e1.h"
#include "proto/yogi_00000c07.h"
#include "proto/yogi_0000c00c.h"
#include "proto/yogi_00debc62.h"

#include <QtDebug>

#include <yogi_core.h>

#include <cassert>
#include <chrono>


TestService* TestService::ms_instance = nullptr;

void TestService::signature_test_thread_fn()
{
    yogi::Logger::set_thread_name("sig test");

    std::unique_lock<std::mutex> lock(m_signatureTestMutex);
    while (!m_terminating) {
        for (auto& fn : m_signatureTestPublishFunctions) {
            fn();
        }

        m_signatureTestCv.wait_for(lock, std::chrono::seconds(1));
    }
}

template <typename Terminal>
void TestService::create_ps_based_test_terminal(std::unique_ptr<Terminal>* terminal, std::unique_ptr<yogi::MessageObserver<Terminal>>* observer,
    std::atomic<bool>* published, const char* name, unsigned rawSignature)
{
    *terminal = std::make_unique<Terminal>(m_leaf, name, yogi::Signature(rawSignature));

    *observer = std::make_unique<yogi::MessageObserver<Terminal>>(**terminal);
    (*observer)->add([=](auto& payload) {
        if (payload.size() == 2 && payload[0] == 12 && payload[1] == 34) {
            *published = true;
        }
    });

    (*observer)->start();
}

template <typename ProtoDescription>
void TestService::create_signature_test_ps_terminal(const char* name,
    std::function<void (typename yogi::PublishSubscribeTerminal<ProtoDescription>::message_type* msg)> fn)
{
    auto terminal = std::make_unique<yogi::PublishSubscribeTerminal<ProtoDescription>>(m_leaf, name);
    m_signatureTestPublishFunctions.push_back([tm = terminal.get(), fn] {
        auto msg = tm->make_message();
        fn(&msg);
        //tm->try_publish(msg);
    });

    auto observer = std::make_unique<yogi::MessageObserver<yogi::PublishSubscribeTerminal<ProtoDescription>>>(*terminal);
    observer->add([tm = terminal.get()](auto msg) {
        tm->try_publish(msg);
    });
    observer->start();
    m_signatureTestObservers.emplace_back(std::move(observer));

    auto binding = std::make_unique<yogi::Binding>(*terminal, name);
    m_signatureTestBindings.emplace_back(std::move(binding));

    m_signatureTestTerminals.emplace_back(std::move(terminal));
}

template <typename ProtoDescription>
void TestService::create_signature_test_sg_terminal(const char* name)
{
    auto terminal = std::make_unique<yogi::ScatterGatherTerminal<ProtoDescription>>(m_leaf, name);

    auto observer = std::make_unique<yogi::MessageObserver<yogi::ScatterGatherTerminal<ProtoDescription>>>(*terminal);
    observer->set([tm = terminal.get()](auto&& request) {
        auto msg = tm->make_gather_message();

        msg.set_timestamp(yogi::Timestamp::now().time_since_epoch().count());

        auto pair1 = msg.add_value();
        pair1->set_first(yogi_00debc62_ns::Tribool::TRUE);
        pair1->set_second(static_cast<std::int8_t>(123));

        auto pair2 = msg.add_value();
        pair2->set_first(yogi_00debc62_ns::Tribool::UNDEFINED);
        pair2->set_second(static_cast<std::int8_t>(101));

        request.respond(msg);

        auto msg2 = tm->make_scatter_message();
        msg2.set_timestamp(123456789);
        auto pair3 = msg2.add_value();
        pair3->set_first(1.23);
        pair3->set_second(std::string("AB"));

        tm->async_scatter_gather(msg2, [](auto& res, auto&& response) {
            return yogi::control_flow::STOP;
        });
    });
    observer->start();
    m_signatureTestObservers.emplace_back(std::move(observer));

    auto binding = std::make_unique<yogi::Binding>(*terminal, name);
    m_signatureTestBindings.emplace_back(std::move(binding));

    m_signatureTestTerminals.emplace_back(std::move(terminal));
}

bool TestService::handle_reset_command(QStringList args)
{
    handle_destroy_yogi_tcp_client_command(QStringList());
    m_yogiTcpServer.reset();

    handle_destroy_test_terminals_command(QStringList());
    handle_destroy_tree_test_terminals_command(QStringList());
    handle_destroy_signature_test_terminals_command(QStringList());
    handle_destroy_producer_terminal_command(QStringList());
    handle_destroy_consumer_terminal_command(QStringList());

    m_psPublished             = false;
    m_cpsPublished            = false;
    m_gatheredRespond         = false;
    m_gatheredIgnore          = false;
    m_consumerPublished       = false;
    m_cachedConsumerPublished = false;
    m_masterPublished         = false;
    m_slavePublished          = false;
    m_cachedMasterPublished   = false;
    m_cachedSlavePublished    = false;
    m_clientResponded         = false;
    m_clientIgnored           = false;

    return true;
}

bool TestService::handle_create_yogi_tcp_factories_command(QStringList args)
{
    if (m_yogiTcpServer) {
        m_yogiTcpServer.reset();
    }

    if (m_yogiTcpClient) {
        m_yogiTcpClient.reset();
    }

    yogi::Configuration serverConfig;
    serverConfig.update(R"(
        {
			"enabled"               : true,
			"address"               : "127.0.0.1",
			"port"                  : 61123,
			"identification"        : "Test identification",
			"accept-identification" : "Test identification",
			"timeout"               : -1.0
		}
    )");

    yogi::Configuration clientConfig;
    clientConfig.update(R"(
        {
			"enabled"               : true,
			"host"                  : "127.0.0.1",
			"port"                  : 61123,
			"identification"        : "Test identification",
			"accept-identification" : "Test identification",
			"timeout"               : -1.0
		}
    )");

    m_yogiTcpServer = std::make_unique<YogiTcpServer>(serverConfig, m_node);
    m_yogiTcpClient = std::make_unique<YogiTcpClient>(clientConfig, m_remoteLeaf);

    return true;
}

bool TestService::handle_destroy_yogi_tcp_client_command(QStringList args)
{
    if (m_yogiTcpClient) {
        m_yogiTcpClient.reset();
        return true;
    }

    return false;
}

bool TestService::handle_create_test_terminals_command(QStringList args)
{
    m_dmTerminal = std::make_unique<yogi::RawDeafMuteTerminal>(m_leaf, "DmTestTerminal", yogi::Signature(0x12345678));

    create_ps_based_test_terminal(&m_psTerminal, &m_psObserver, &m_psPublished, "PsTestTerminal", 0x12345678);
    create_ps_based_test_terminal(&m_cpsTerminal, &m_cpsObserver, &m_cpsPublished, "CpsTestTerminal", 0x12345678);

    m_sgTerminal = std::make_unique<yogi::RawScatterGatherTerminal>(m_leaf, "SgTestTerminal", yogi::Signature(0x12345678));
    m_sgObserver = std::make_unique<yogi::MessageObserver<yogi::RawScatterGatherTerminal>>(*m_sgTerminal);
    m_sgObserver->set([=](auto&& msg) {
        if (msg.data().size() == 2 && msg.data()[0] == 12 && msg.data()[1] == 34) {
            const char response[] = {56, 78};
            msg.respond(response, sizeof(response));
        }
    });
    m_sgObserver->start();

    m_producerTerminal = std::make_unique<yogi::RawProducerTerminal>(m_leaf, "PcTestTerminal", yogi::Signature(0x12345678));
    create_ps_based_test_terminal(&m_consumerTerminal, &m_consumerObserver, &m_consumerPublished, "PcTestTerminal", 0x12345678);

    m_cachedProducerTerminal = std::make_unique<yogi::RawCachedProducerTerminal>(m_leaf, "CpcTestTerminal", yogi::Signature(0x12345678));
    create_ps_based_test_terminal(&m_cachedConsumerTerminal, &m_cachedConsumerObserver, &m_cachedConsumerPublished, "CpcTestTerminal", 0x12345678);

    create_ps_based_test_terminal(&m_masterTerminal, &m_masterObserver, &m_masterPublished, "MsTestTerminal", 0x12345678);
    create_ps_based_test_terminal(&m_slaveTerminal, &m_slaveObserver, &m_slavePublished, "MsTestTerminal", 0x12345678);

    create_ps_based_test_terminal(&m_cachedMasterTerminal, &m_cachedMasterObserver, &m_cachedMasterPublished, "CmsTestTerminal", 0x12345678);
    create_ps_based_test_terminal(&m_cachedSlaveTerminal, &m_cachedSlaveObserver, &m_cachedSlavePublished, "CmsTestTerminal", 0x12345678);

    m_serviceTerminal = std::make_unique<yogi::RawServiceTerminal>(m_leaf, "ScTestTerminal", yogi::Signature(0x12345678));
    m_serviceObserver = std::make_unique<yogi::MessageObserver<yogi::RawServiceTerminal>>(*m_serviceTerminal);
    m_serviceObserver->set([=](auto&& msg) {
        if (msg.data().size() == 2 && msg.data()[0] == 12 && msg.data()[1] == 34) {
            const char response[] = {56, 78};
            msg.respond(response, sizeof(response));
        }
    });
    m_serviceObserver->start();

    m_clientTerminal = std::make_unique<yogi::RawClientTerminal>(m_leaf, "ScTestTerminal", yogi::Signature(0x12345678));

    m_psBinding = std::make_unique<yogi::Binding>(*m_psTerminal, "PsTestTerminal");
    m_cpsBinding = std::make_unique<yogi::Binding>(*m_cpsTerminal, "CpsTestTerminal");
    m_sgBinding = std::make_unique<yogi::Binding>(*m_sgTerminal, "SgTestTerminal");

    return true;
}

bool TestService::handle_destroy_test_terminals_command(QStringList args)
{
    m_psObserver              .reset();
    m_cpsObserver             .reset();
    m_sgObserver              .reset();
    m_consumerObserver        .reset();
    m_cachedConsumerObserver  .reset();
    m_masterObserver          .reset();
    m_slaveObserver           .reset();
    m_cachedMasterObserver    .reset();
    m_cachedSlaveObserver     .reset();
    m_serviceObserver         .reset();

    m_psBinding               .reset();
    m_cpsBinding              .reset();
    m_sgBinding               .reset();

    m_dmTerminal              .reset();
    m_psTerminal              .reset();
    m_cpsTerminal             .reset();
    m_sgTerminal              .reset();
    m_producerTerminal        .reset();
    m_consumerTerminal        .reset();
    m_cachedProducerTerminal  .reset();
    m_cachedConsumerTerminal  .reset();
    m_masterTerminal          .reset();
    m_slaveTerminal           .reset();
    m_cachedMasterTerminal    .reset();
    m_cachedSlaveTerminal     .reset();
    m_serviceTerminal         .reset();
    m_clientTerminal          .reset();
    m_separateProducerTerminal.reset();
    m_separateConsumerTerminal.reset();

    return true;
}

bool TestService::handle_ps_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_psTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_ps_check_published_command(QStringList args)
{
    return m_psPublished;
}

bool TestService::handle_sg_scatter_command(QStringList args)
{
    const char scatBuf[] = {56, 78};
    return !!m_sgTerminal->try_async_scatter_gather(scatBuf, sizeof(scatBuf), [=](auto& res, auto&& msg) {
        if (res) {
            if (msg.flags() & yogi::gather_flags::IGNORED) {
                this->m_gatheredIgnore = true;
            }
            else if (msg.data().size() == 2 && msg.data()[0] == 12 && msg.data()[1] == 34) {
                this->m_gatheredRespond = true;
            }
        }
        return yogi::control_flow::STOP;
    });
}

bool TestService::handle_sg_check_gathered_respond_command(QStringList args)
{
    return m_gatheredRespond;
}

bool TestService::handle_sg_check_gathered_ignore_command(QStringList args)
{
    return m_gatheredIgnore;
}

bool TestService::handle_cps_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_cpsTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_cps_check_published_command(QStringList args)
{
    return m_cpsPublished;
}

bool TestService::handle_producer_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_producerTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_consumer_check_published_command(QStringList args)
{
    return m_consumerPublished;
}

bool TestService::handle_cached_producer_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_cachedProducerTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_cached_consumer_check_published_command(QStringList args)
{
    return m_cachedConsumerPublished;
}

bool TestService::handle_master_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_masterTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_master_check_published_command(QStringList args)
{
    return m_masterPublished;
}

bool TestService::handle_slave_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_slaveTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_slave_check_published_command(QStringList args)
{
    return m_slavePublished;
}

bool TestService::handle_cached_master_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_cachedMasterTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_cached_master_check_published_command(QStringList args)
{
    return m_cachedMasterPublished;
}

bool TestService::handle_cached_slave_publish_command(QStringList args)
{
    const char buffer[] = {56, 78};
    return m_cachedSlaveTerminal->try_publish(buffer, sizeof(buffer));
}

bool TestService::handle_cached_slave_check_published_command(QStringList args)
{
    return m_cachedSlavePublished;
}

bool TestService::handle_sc_request_command(QStringList args)
{
    const char scatBuf[] = {56, 78};
    return !!m_clientTerminal->try_async_request(scatBuf, sizeof(scatBuf), [=](auto& res, auto&& msg) {
        if (res) {
            if (msg.flags() & yogi::gather_flags::IGNORED) {
                this->m_clientIgnored = true;
            }
            else if (msg.data().size() == 2 && msg.data()[0] == 12 && msg.data()[1] == 34) {
                this->m_clientResponded = true;
            }
        }
        return yogi::control_flow::STOP;
    });
}

bool TestService::handle_sc_check_responded_command(QStringList args)
{
    return m_clientResponded;
}

bool TestService::handle_sc_check_ignored_command(QStringList args)
{
    return m_clientIgnored;
}

bool TestService::handle_create_tree_test_terminals_command(QStringList args)
{
    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawDeafMuteTerminal>(
        m_leaf, "/Power Module/Alive", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "/Power Module/Motor 1/Voltage", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "/Power Module/Motor 1/Current", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawCachedPublishSubscribeTerminal>(
        m_leaf, "/Power Module/Motor 1/Park Brakes Active", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "/Power Module/Motor 2/Voltage", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "/Power Module/Motor 2/Current", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawCachedPublishSubscribeTerminal>
        (m_leaf, "/Power Module/Motor 2/Park Brakes Active", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawScatterGatherTerminal>(
        m_leaf, "/Power System Up", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawScatterGatherTerminal>(
        m_leaf, "/Power System Off", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawDeafMuteTerminal>(
        m_leaf, "Remote/Joystick/Calibration", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "Remote/Joystick/Calibration/Horizontal", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawScatterGatherTerminal>(
        m_leaf, "Remote/Joystick/Calibration/Horizontal", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "Remote/Joystick/Calibration/Vertical", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawScatterGatherTerminal>(
        m_leaf, "Remote/Joystick/Calibration/Vertical", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "Remote/Joystick/Deflection/X", yogi::Signature(0x01000000)));

    m_treeTestTerminals.emplace_back(std::make_unique<yogi::RawPublishSubscribeTerminal>(
        m_leaf, "Remote/Joystick/Deflection/Y", yogi::Signature(0x01000000)));

    return true;
}

bool TestService::handle_destroy_tree_test_terminals_command(QStringList args)
{
    m_treeTestTerminals.clear();
    return true;
}

bool TestService::handle_create_signature_test_terminals_command(QStringList args)
{
    std::lock_guard<std::mutex> lock(m_signatureTestMutex);

    create_signature_test_ps_terminal<yogi_00000000>("/PS void", [](auto* msg) {
    });

    create_signature_test_ps_terminal<yogi_00000001>("/PS bool", [](auto* msg) {
        static bool val;
        val = !val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000002>("/PS tribool", [](auto* msg) {
        static int val;
        val = (val + 1) % 3;
        msg->set_value(static_cast<yogi_00000002_ns::Tribool>(val));
    });

    create_signature_test_ps_terminal<yogi_00000003>("/PS int8", [](auto* msg) {
        static std::int8_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000004>("/PS uint8", [](auto* msg) {
        static std::uint8_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000005>("/PS int16", [](auto* msg) {
        static std::int16_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000006>("/PS uint16", [](auto* msg) {
        static std::uint16_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000007>("/PS int32", [](auto* msg) {
        static std::int32_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000008>("/PS uint32", [](auto* msg) {
        static std::uint32_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000009>("/PS int64", [](auto* msg) {
        static std::int64_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_0000000a>("/PS uint64", [](auto* msg) {
        static std::uint64_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_0000000b>("/PS float", [](auto* msg) {
        static float val;
        val += 0.1;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_0000000c>("/PS double", [](auto* msg) {
        static double val;
        val += 0.1;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_0000000d>("/PS string", [](auto* msg) {
        static const char one[] = "One";
        static const char two[] = "Two";
        static const char* val = one;
        val = val == one ? two : one;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_0000000e>("/PS JSON", [](auto* msg) {
        static const char one[] = "{\"x\": 123}";
        static const char two[] = "{\"bla\": \"hello\"}";
        static const char* val = one;
        val = val == one ? two : one;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_0000000f>("/PS BLOB", [](auto* msg) {
        static const char one[] = "123";
        static const char two[] = "456";
        static const char* val = one;
        val = val == one ? two : one;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_00000010>("/PS Timestamp", [](auto* msg) {
        static std::uint64_t val;
        ++val;
        msg->set_value(val);
    });

    create_signature_test_ps_terminal<yogi_000000e1>("/PS pair<bool, int32>", [](auto* msg) {
        static yogi_000000e1_ns::PublishMessage::Pair val;
        val.set_first(!val.first());
        val.set_second(val.second() + 1);
        msg->set_allocated_value(new decltype(val)(val));
    });

    create_signature_test_ps_terminal<yogi_00000c07>("/PS list<int32> with timestamp", [](auto* msg) {
        static bool x;
        x = !x;
        if (x) {
            msg->add_value(123);
            msg->add_value(456);
        }
        else {
            msg->add_value(789);
        }
        msg->set_timestamp(msg->timestamp() + 1);
    });

    create_signature_test_sg_terminal<yogi_00debc62>("/SG Scatter String Gather int32");

    return true;
}

bool TestService::handle_destroy_signature_test_terminals_command(QStringList args)
{
    std::lock_guard<std::mutex> lock(m_signatureTestMutex);
    m_signatureTestPublishFunctions.clear();
    m_signatureTestBindings        .clear();
    m_signatureTestObservers       .clear();
    m_signatureTestTerminals       .clear();

    return true;
}

bool TestService::handle_create_producer_terminal_command(QStringList args)
{
    m_separateProducerTerminal = std::make_unique<yogi::RawProducerTerminal>(m_leaf, "ProducerTestTerminal", yogi::Signature(0));
	return true;
}

bool TestService::handle_destroy_producer_terminal_command(QStringList args)
{
    m_separateProducerTerminal.reset();
	return true;
}

bool TestService::handle_create_consumer_terminal_command(QStringList args)
{
    m_separateConsumerTerminal = std::make_unique<yogi::RawConsumerTerminal>(m_leaf, "ConsumerTestTerminal", yogi::Signature(0));
    return true;
}

bool TestService::handle_destroy_consumer_terminal_command(QStringList args)
{
    m_separateConsumerTerminal.reset();
    return true;
}

TestService::TestService(yogi::Node& node)
: m_logger("Test Service")
, m_node(node)
, m_leaf(node.scheduler())
, m_remoteLeaf(node.scheduler())
, m_connection(m_node, m_leaf)
, m_psPublished(false)
, m_cpsPublished(false)
, m_gatheredRespond(false)
, m_gatheredIgnore(false)
, m_consumerPublished(false)
, m_cachedConsumerPublished(false)
, m_masterPublished(false)
, m_slavePublished(false)
, m_cachedMasterPublished(false)
, m_cachedSlavePublished(false)
, m_clientResponded(false)
, m_clientIgnored(false)
{
    if (!yogi::ProcessInterface::config().get<bool>("test-service.enabled")) {
        YOGI_LOG_DEBUG(m_logger, "Disabled test service");
        return;
    }

    assert (ms_instance == nullptr);
    ms_instance = this;

    m_terminating = false;
    m_signatureTestThread = std::thread(&TestService::signature_test_thread_fn, this);

    YOGI_LOG_INFO(m_logger, "Test service enabled");
}

TestService::~TestService()
{
    handle_destroy_signature_test_terminals_command(QStringList());

    m_terminating = true;
    m_signatureTestCv.notify_all();

    if (m_signatureTestThread.joinable()) {
        m_signatureTestThread.join();
    }

    ms_instance = nullptr;
    handle_reset_command(QStringList());
}

bool TestService::active()
{
    return ms_instance != nullptr;
}

TestService& TestService::instance()
{
    assert (ms_instance != nullptr);
    return *ms_instance;
}

bool TestService::handle_command(QString command)
{
    YOGI_LOG_INFO(m_logger, "Received command: " << command);
    auto args = command.split(' ').mid(1);

    bool ok = false;
    if (command.startsWith("reset")) {
        ok = handle_reset_command(args);
    }
    else if (command.startsWith("createYogiTcpFactories")) {
        ok = handle_create_yogi_tcp_factories_command(args);
    }
    else if (command.startsWith("destroyYogiTcpClient")) {
        ok = handle_destroy_yogi_tcp_client_command(args);
    }
    else if (command.startsWith("createTestTerminals")) {
        ok = handle_create_test_terminals_command(args);
    }
    else if (command.startsWith("destroyTestTerminals")) {
        ok = handle_destroy_test_terminals_command(args);
    }
    else if (command.startsWith("psPublish")) {
        ok = handle_ps_publish_command(args);
    }
    else if (command.startsWith("psCheckPublished")) {
        ok = handle_ps_check_published_command(args);
    }
    else if (command.startsWith("sgScatter")) {
        ok = handle_sg_scatter_command(args);
    }
    else if (command.startsWith("sgCheckGatheredRespond")) {
        ok = handle_sg_check_gathered_respond_command(args);
    }
    else if (command.startsWith("sgCheckGatheredIgnore")) {
        ok = handle_sg_check_gathered_ignore_command(args);
    }
    else if (command.startsWith("cpsPublish")) {
        ok = handle_cps_publish_command(args);
    }
    else if (command.startsWith("cpsCheckPublished")) {
        ok = handle_cps_check_published_command(args);
    }
    else if (command.startsWith("producerPublish")) {
        ok = handle_producer_publish_command(args);
    }
    else if (command.startsWith("consumerCheckPublished")) {
        ok = handle_consumer_check_published_command(args);
    }
    else if (command.startsWith("cachedProducerPublish")) {
        ok = handle_cached_producer_publish_command(args);
    }
    else if (command.startsWith("cachedConsumerCheckPublished")) {
        ok = handle_cached_consumer_check_published_command(args);
    }
    else if (command.startsWith("masterPublish")) {
        ok = handle_master_publish_command(args);
    }
    else if (command.startsWith("masterCheckPublished")) {
        ok = handle_master_check_published_command(args);
    }
    else if (command.startsWith("slavePublish")) {
        ok = handle_slave_publish_command(args);
    }
    else if (command.startsWith("slaveCheckPublished")) {
        ok = handle_slave_check_published_command(args);
    }
    else if (command.startsWith("cachedMasterPublish")) {
        ok = handle_cached_master_publish_command(args);
    }
    else if (command.startsWith("cachedMasterCheckPublished")) {
        ok = handle_cached_master_check_published_command(args);
    }
    else if (command.startsWith("cachedSlavePublish")) {
        ok = handle_cached_slave_publish_command(args);
    }
    else if (command.startsWith("cachedSlaveCheckPublished")) {
        ok = handle_cached_slave_check_published_command(args);
    }
    else if (command.startsWith("scRequest")) {
        ok = handle_sc_request_command(args);
    }
    else if (command.startsWith("scCheckResponded")) {
        ok = handle_sc_check_responded_command(args);
    }
    else if (command.startsWith("scCheckIgnored")) {
        ok = handle_sc_check_ignored_command(args);
    }
    else if (command.startsWith("createTreeTestTerminals")) {
        ok = handle_create_tree_test_terminals_command(args);
    }
    else if (command.startsWith("destroyTreeTestTerminals")) {
        ok = handle_destroy_tree_test_terminals_command(args);
    }
    else if (command.startsWith("createSignatureTestTerminals")) {
        ok = handle_create_signature_test_terminals_command(args);
    }
    else if (command.startsWith("destroySignatureTestTerminals")) {
        ok = handle_destroy_signature_test_terminals_command(args);
    }
	else if (command.startsWith("createProducerTerminal")) {
		ok = handle_create_producer_terminal_command(args);
	}
	else if (command.startsWith("destroyProducerTerminal")) {
		ok = handle_destroy_producer_terminal_command(args);
	}
    else if (command.startsWith("createConsumerTerminal")) {
        ok = handle_create_consumer_terminal_command(args);
    }
    else if (command.startsWith("destroyConsumerTerminal")) {
        ok = handle_destroy_consumer_terminal_command(args);
    }
    else {
        YOGI_LOG_ERROR("Invalid command received: " << command);
    }

    return ok;
}
