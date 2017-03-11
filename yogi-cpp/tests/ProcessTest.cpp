#include <gtest/gtest.h>

#include "../yogi_cpp/process.hpp"
#include "../yogi_cpp/leaf.hpp"
#include "../yogi_cpp/connection.hpp"
#include "../yogi_cpp/binding.hpp"
#include "../yogi_cpp/terminals.hpp"
#include "../yogi_cpp/configuration.hpp"
#include "../yogi_cpp/internal/proto/yogi_00000001.h"
#include "../yogi_cpp/internal/proto/yogi_0000040d.h"
using namespace yogi;

#include <atomic>
#include <string>
#include <vector>


struct ProcessTest : public testing::Test
{
    ProcessInterface pi;
    Leaf             leaf;
    LocalConnection  conn;

    CachedConsumerTerminal<internal::yogi_00000001> operationalTerminal;
    CachedConsumerTerminal<internal::yogi_0000040d> errorsTerminal;
    CachedConsumerTerminal<internal::yogi_0000040d> warningsTerminal;

    ProcessTest()
    : pi(Configuration())
    , leaf(pi.scheduler())
    , conn(leaf, pi.leaf())
    , operationalTerminal(leaf, pi.location() / "Process/Operational")
    , errorsTerminal     (leaf, pi.location() / "Process/Errors")
    , warningsTerminal   (leaf, pi.location() / "Process/Warnings")
    {
        while (operationalTerminal.get_binding_state() == RELEASED);
        await_cached_message(operationalTerminal);

        while (errorsTerminal.get_binding_state() == RELEASED);
        await_cached_message(errorsTerminal);

        while (warningsTerminal.get_binding_state() == RELEASED);
        await_cached_message(warningsTerminal);
    }

    template <typename Terminal>
    void await_cached_message(Terminal& terminal)
    {
        while (true) {
            try {
                terminal.get_cached_message();
                break;
            }
            catch(...) {
            }
        }
    }

    template <typename Fn>
    bool monitor_operational_state(Fn fn)
    {
        std::atomic<bool> called{false};
        bool state;
        operationalTerminal.async_receive_message([&](auto& res, auto&& msg, auto cached) {
            EXPECT_EQ(Success(), res);
            EXPECT_FALSE(cached);
            state = msg.value();
            called = true;
        });

        fn();

        while (!called);
        EXPECT_EQ(state, pi.operational());
        return state;
    }

    template <typename Fn>
    std::vector<std::string> monitor_anomalies(CachedConsumerTerminal<internal::yogi_0000040d>& terminal, Fn fn)
    {
        std::atomic<bool> called{false};
        std::vector<std::string> anomalies;
        terminal.async_receive_message([&](auto& res, auto&& msg, auto cached) {
            EXPECT_EQ(Success(), res);
            EXPECT_FALSE(cached);

            for (int i = 0; i < msg.value_size(); ++i) {
                anomalies.push_back(msg.value(i));
            }

            called = true;
        });

        fn();

        while (!called);
        return anomalies;
    }
};

TEST_F(ProcessTest, InitialOperationalState)
{
    auto msg = operationalTerminal.get_cached_message();
    EXPECT_TRUE(msg.value());
}

TEST_F(ProcessTest, OperationalConditions)
{
    std::unique_ptr<ManualOperationalCondition> oc1;
    std::unique_ptr<ManualOperationalCondition> oc2;

    EXPECT_FALSE(monitor_operational_state([&] {
        oc1 = std::make_unique<ManualOperationalCondition>("OC 1");
    }));

    EXPECT_TRUE(monitor_operational_state([&] {
        oc1->set();
    }));

    EXPECT_FALSE(monitor_operational_state([&] {
        oc2 = std::make_unique<ManualOperationalCondition>("OC 2");
    }));

    EXPECT_TRUE(monitor_operational_state([&] {
        oc2->set();
    }));

    EXPECT_FALSE(monitor_operational_state([&] {
        oc2->clear();
    }));

    EXPECT_TRUE(monitor_operational_state([&] {
        oc2.reset();
    }));

    EXPECT_FALSE(monitor_operational_state([&] {
        oc1->clear();
    }));

    EXPECT_TRUE(monitor_operational_state([&] {
        oc1.reset();
    }));
}

TEST_F(ProcessTest, OperationalObserver)
{
    ManualOperationalCondition moc("Test Condition");
    OperationalObserver        observer;

    int i = 0;
    auto id = observer.add([&](operational_flag operational) {
        switch (i++) {
        case 0: EXPECT_FALSE(operational); break;
        case 1: EXPECT_TRUE( operational); break;
        case 2: EXPECT_FALSE(operational); break;
        }
    });

    int j = 0;
    observer.add([&](operational_flag operational) {
        switch (j++) {
        case 0: EXPECT_FALSE(operational); break;
        case 1: EXPECT_TRUE( operational); break;
        case 2: EXPECT_FALSE(operational); break;
        case 3: EXPECT_TRUE( operational); break;
        }
    });

    moc.set();
    moc.clear();

    EXPECT_EQ(0, i);
    EXPECT_EQ(0, j);

    observer.start();

    EXPECT_EQ(1, i);
    EXPECT_EQ(1, j);

    moc.set();

    EXPECT_EQ(2, i);
    EXPECT_EQ(2, j);

    observer.remove(id);

    moc.clear();

    EXPECT_EQ(2, i);
    EXPECT_EQ(3, j);

    observer.stop();

    moc.set();

    EXPECT_EQ(2, i);
    EXPECT_EQ(3, j);
}

TEST_F(ProcessTest, InitialErrorsAndWarnings)
{
    auto msg = errorsTerminal.get_cached_message();
    EXPECT_EQ(0, msg.value_size());

    msg = warningsTerminal.get_cached_message();
    EXPECT_EQ(0, msg.value_size());
}

TEST_F(ProcessTest, Errors)
{
    Error err1("Out of coffee");
    auto err2 = std::make_unique<Error>("Out of soy milk");

    // set error
    EXPECT_EQ((std::vector<std::string>{
        err1.message()
    }), monitor_anomalies(errorsTerminal, [&] {
        err1.set();
    }));

    // set another error
    EXPECT_EQ((std::vector<std::string>{
        err1.message(),
        err2->message()
    }), monitor_anomalies(errorsTerminal, [&] {
        err2->set();
    }));

    // clear an error
    EXPECT_EQ((std::vector<std::string>{
        err2->message()
    }), monitor_anomalies(errorsTerminal, [&] {
        err1.clear();
    }));

    // destroy an error
    EXPECT_EQ((std::vector<std::string>{
    }), monitor_anomalies(errorsTerminal, [&] {
        err2.reset();
    }));

    // set a timed error
    EXPECT_EQ((std::vector<std::string>{
        err1.message()
    }), monitor_anomalies(errorsTerminal, [&] {
        err1.set(std::chrono::milliseconds(1));
    }));

    while (errorsTerminal.get_cached_message().value_size() != 0);
    EXPECT_EQ(0, errorsTerminal.get_cached_message().value_size());
}

TEST_F(ProcessTest, Warnings)
{
    Warning warn1("Out of coffee");
    auto warn2 = std::make_unique<Warning>("Out of soy milk");

    // set error
    EXPECT_EQ((std::vector<std::string>{
        warn1.message()
    }), monitor_anomalies(warningsTerminal, [&] {
            warn1.set();
    }));

    // set another error
    EXPECT_EQ((std::vector<std::string>{
        warn1.message(),
            warn2->message()
    }), monitor_anomalies(warningsTerminal, [&] {
            warn2->set();
    }));

    // clear an error
    EXPECT_EQ((std::vector<std::string>{
        warn2->message()
    }), monitor_anomalies(warningsTerminal, [&] {
            warn1.clear();
    }));

    // destroy an error
    EXPECT_EQ((std::vector<std::string>{
    }), monitor_anomalies(warningsTerminal, [&] {
        warn2.reset();
    }));

    // set a timed error
    EXPECT_EQ((std::vector<std::string>{
        warn1.message()
    }), monitor_anomalies(warningsTerminal, [&] {
            warn1.set(std::chrono::milliseconds(1));
    }));

    while (warningsTerminal.get_cached_message().value_size() != 0);
    EXPECT_EQ(0, warningsTerminal.get_cached_message().value_size());
}

TEST_F(ProcessTest, Dependency)
{
    ConsumerTerminal<internal::yogi_00000001>         tmA1(pi.leaf(), "1");
    PublishSubscribeTerminal<internal::yogi_00000001> tmA2(pi.leaf(), "2");
    Binding                                               bd(tmA2, "2");

    Dependency dep("My Dependency", tmA1, bd);
    EXPECT_FALSE(pi.operational());

    ProducerTerminal<internal::yogi_00000001> tmB1(leaf, "1");
    EXPECT_FALSE(pi.operational());

    auto tmB2 = std::make_unique<PublishSubscribeTerminal<internal::yogi_00000001>>(leaf, "2");
    while (!pi.operational());
    EXPECT_TRUE(pi.operational());

    tmB2.reset();
    while (pi.operational());
    EXPECT_FALSE(pi.operational());
}

TEST_F(ProcessTest, ProcessDependency)
{
    ConsumerTerminal<internal::yogi_00000001> tmA("A");

    ProcessDependency dep("/My Process", tmA);
    EXPECT_FALSE(pi.operational());

    ProducerTerminal<internal::yogi_00000001> tmB(leaf, "/A");
    EXPECT_FALSE(pi.operational());

    CachedProducerTerminal<internal::yogi_00000001> tmO(leaf, "/My Process/Process/Operational");
    EXPECT_FALSE(pi.operational());

    auto msg = tmO.make_message();
    msg.set_value(true);
    tmO.try_publish(msg);

    while (!pi.operational());
    EXPECT_TRUE(pi.operational());

    msg.set_value(false);
    tmO.try_publish(msg);

    while (pi.operational());
    EXPECT_FALSE(pi.operational());
}
