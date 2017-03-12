#ifndef TESTSERVICE_HPP
#define TESTSERVICE_HPP

#include "YogiTcpServer.hpp"
#include "YogiTcpClient.hpp"

#include <yogi_cpp.hpp>

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>


class TestService
{
private:
    static TestService* ms_instance;

    yogi::Logger m_logger;

    std::atomic<bool> m_terminating;

    yogi::Node&           m_node;
    yogi::Leaf            m_leaf;
    yogi::Leaf            m_remoteLeaf;
    yogi::LocalConnection m_connection;

    std::unique_ptr<yogi::RawDeafMuteTerminal>               m_dmTerminal;
    std::unique_ptr<yogi::RawPublishSubscribeTerminal>       m_psTerminal;
    std::unique_ptr<yogi::RawCachedPublishSubscribeTerminal> m_cpsTerminal;
    std::unique_ptr<yogi::RawScatterGatherTerminal>          m_sgTerminal;
    std::unique_ptr<yogi::RawProducerTerminal>               m_producerTerminal;
    std::unique_ptr<yogi::RawConsumerTerminal>               m_consumerTerminal;
    std::unique_ptr<yogi::RawCachedProducerTerminal>         m_cachedProducerTerminal;
    std::unique_ptr<yogi::RawCachedConsumerTerminal>         m_cachedConsumerTerminal;
    std::unique_ptr<yogi::RawMasterTerminal>                 m_masterTerminal;
    std::unique_ptr<yogi::RawSlaveTerminal>                  m_slaveTerminal;
    std::unique_ptr<yogi::RawCachedMasterTerminal>           m_cachedMasterTerminal;
    std::unique_ptr<yogi::RawCachedSlaveTerminal>            m_cachedSlaveTerminal;
    std::unique_ptr<yogi::RawServiceTerminal>                m_serviceTerminal;
    std::unique_ptr<yogi::RawClientTerminal>                 m_clientTerminal;
    std::unique_ptr<yogi::RawProducerTerminal>               m_separateProducerTerminal;
    std::unique_ptr<yogi::RawConsumerTerminal>               m_separateConsumerTerminal;

    std::unique_ptr<yogi::Binding> m_psBinding;
    std::unique_ptr<yogi::Binding> m_cpsBinding;
    std::unique_ptr<yogi::Binding> m_sgBinding;

    std::unique_ptr<yogi::MessageObserver<yogi::RawPublishSubscribeTerminal>>       m_psObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawCachedPublishSubscribeTerminal>> m_cpsObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawScatterGatherTerminal>>          m_sgObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawConsumerTerminal>>               m_consumerObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawCachedConsumerTerminal>>         m_cachedConsumerObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawMasterTerminal>>                 m_masterObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawSlaveTerminal>>                  m_slaveObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawCachedMasterTerminal>>           m_cachedMasterObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawCachedSlaveTerminal>>            m_cachedSlaveObserver;
    std::unique_ptr<yogi::MessageObserver<yogi::RawServiceTerminal>>                m_serviceObserver;

    std::atomic<bool> m_psPublished;
    std::atomic<bool> m_cpsPublished;
    std::atomic<bool> m_gatheredRespond;
    std::atomic<bool> m_gatheredIgnore;
    std::atomic<bool> m_consumerPublished;
    std::atomic<bool> m_cachedConsumerPublished;
    std::atomic<bool> m_masterPublished;
    std::atomic<bool> m_slavePublished;
    std::atomic<bool> m_cachedMasterPublished;
    std::atomic<bool> m_cachedSlavePublished;
    std::atomic<bool> m_clientResponded;
    std::atomic<bool> m_clientIgnored;

    std::unique_ptr<YogiTcpServer> m_yogiTcpServer;
    std::unique_ptr<YogiTcpClient> m_yogiTcpClient;

    std::vector<std::unique_ptr<yogi::Terminal>> m_treeTestTerminals;
    std::vector<std::unique_ptr<yogi::Terminal>> m_signatureTestTerminals;
    std::vector<std::unique_ptr<yogi::Observer>> m_signatureTestObservers;
    std::vector<std::unique_ptr<yogi::Binding>>  m_signatureTestBindings;
    std::vector<std::function<void ()>>          m_signatureTestPublishFunctions;
    std::thread                                  m_signatureTestThread;
    std::mutex                                   m_signatureTestMutex;
    std::condition_variable                      m_signatureTestCv;

private:
    void signature_test_thread_fn();

    template <typename Terminal>
    void create_ps_based_test_terminal(std::unique_ptr<Terminal>* terminal, std::unique_ptr<yogi::MessageObserver<Terminal>>* observer,
        std::atomic<bool>* published, const char* name, unsigned rawSignature);

    template <typename ProtoDescription>
    void create_signature_test_ps_terminal(const char* name,
        std::function<void (typename yogi::PublishSubscribeTerminal<ProtoDescription>::message_type* msg)> fn);

    template <typename ProtoDescription>
    void create_signature_test_sg_terminal(const char* name);

    bool handle_reset_command(QStringList args);
    bool handle_create_yogi_tcp_factories_command(QStringList args);
    bool handle_destroy_yogi_tcp_client_command(QStringList args);
    bool handle_create_test_terminals_command(QStringList args);
    bool handle_destroy_test_terminals_command(QStringList args);
    bool handle_ps_publish_command(QStringList args);
    bool handle_ps_check_published_command(QStringList args);
    bool handle_sg_scatter_command(QStringList args);
    bool handle_sg_check_gathered_respond_command(QStringList args);
    bool handle_sg_check_gathered_ignore_command(QStringList args);
    bool handle_cps_publish_command(QStringList args);
    bool handle_cps_check_published_command(QStringList args);
    bool handle_producer_publish_command(QStringList args);
    bool handle_consumer_check_published_command(QStringList args);
    bool handle_cached_producer_publish_command(QStringList args);
    bool handle_cached_consumer_check_published_command(QStringList args);
    bool handle_master_publish_command(QStringList args);
    bool handle_master_check_published_command(QStringList args);
    bool handle_slave_publish_command(QStringList args);
    bool handle_slave_check_published_command(QStringList args);
    bool handle_cached_master_publish_command(QStringList args);
    bool handle_cached_master_check_published_command(QStringList args);
    bool handle_cached_slave_publish_command(QStringList args);
    bool handle_cached_slave_check_published_command(QStringList args);
    bool handle_sc_request_command(QStringList args);
    bool handle_sc_check_responded_command(QStringList args);
    bool handle_sc_check_ignored_command(QStringList args);
    bool handle_create_tree_test_terminals_command(QStringList args);
    bool handle_destroy_tree_test_terminals_command(QStringList args);
    bool handle_create_signature_test_terminals_command(QStringList args);
    bool handle_destroy_signature_test_terminals_command(QStringList args);
	bool handle_create_producer_terminal_command(QStringList args);
	bool handle_destroy_producer_terminal_command(QStringList args);
    bool handle_create_consumer_terminal_command(QStringList args);
    bool handle_destroy_consumer_terminal_command(QStringList args);

public:
    TestService(yogi::Node& node);
    ~TestService();
    static bool active();
    static TestService& instance();

    bool handle_command(QString command);
};

#endif // TESTSERVICE_HPP
