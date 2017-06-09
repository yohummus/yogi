#include <gtest/gtest.h>

#include "../yogi/logging.hpp"
#include "../yogi/configuration.hpp"
#include "../yogi/process.hpp"
#include "../yogi/leaf.hpp"
#include "../yogi/connection.hpp"
#include "../yogi/terminals.hpp"
#include "../yogi/internal/proto/yogi_00004004.h"
#include "../yogi/internal/proto/yogi_000009cd.h"
using namespace yogi;

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
namespace pt = boost::property_tree;

#include <sstream>


struct LoggingTest : public testing::Test
{
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
    std::uint8_t monitor_verbosity(CachedSlaveTerminal<internal::yogi_00004004>& terminal, Fn fn)
    {
        std::atomic<bool> called{false};
        std::uint8_t verb;
        terminal.async_receive_message([&](auto& res, auto&& msg, auto cached) {
            EXPECT_EQ(Success(), res);
            EXPECT_FALSE(cached);
            verb = static_cast<std::uint8_t>(msg.value());
            called = true;
        });

        fn();

        while (!called);
        return verb;
    }

    template <typename Fn>
    ConsumerTerminal<internal::yogi_000009cd>::message_type monitor_log(ConsumerTerminal<internal::yogi_000009cd>& terminal, Fn fn)
    {
        std::atomic<bool> called{false};
        ConsumerTerminal<internal::yogi_000009cd>::message_type logMsg;
        terminal.async_receive_message([&](auto& res, auto&& msg) {
            EXPECT_EQ(Success(), res);
            logMsg = msg;
            called = true;
        });

        fn();

        while (!called);
        return logMsg;
    }
};

TEST_F(LoggingTest, AppLogger)
{
    YOGI_LOG_INFO("Hey dude " << 123 << "!");
}

TEST_F(LoggingTest, ComponentLogger)
{
    Logger logger("My Component");
    YOGI_LOG_INFO(logger, "Hey dude " << 123 << "!");
    YOGI_LOG(INFO, logger, "Hello" << " you");
}

TEST_F(LoggingTest, ThreadNames)
{
    Logger::set_thread_name("Alex");
    YOGI_LOG_FATAL("My thread name should be 'Alex'");

    Logger::set_thread_name("Carol", true);
    YOGI_LOG_FATAL("My thread name should be 'Carol' with a number as a suffix");

    Logger::set_thread_name("T", true);
}

TEST_F(LoggingTest, NonColourised)
{
    Logger::app_logger().set_stdout_verbosity(verbosity::TRACE);

    YOGI_LOG_FATAL("FATAL");
    YOGI_LOG_ERROR("ERROR");
    YOGI_LOG_WARNING("WARNING");
    YOGI_LOG_INFO("INFO");
    YOGI_LOG_DEBUG("DEBUG");
    YOGI_LOG_TRACE("TRACE");
}

TEST_F(LoggingTest, Colourised)
{
    Logger::set_colourised_stdout(true);
    Logger::app_logger().set_stdout_verbosity(verbosity::TRACE);

    YOGI_LOG_FATAL("FATAL");
    YOGI_LOG_ERROR("ERROR");
    YOGI_LOG_WARNING("WARNING");
    YOGI_LOG_INFO("INFO");
    YOGI_LOG_DEBUG("DEBUG");
    YOGI_LOG_TRACE("TRACE");
}

TEST_F(LoggingTest, EffectiveVerbosity)
{
    Logger().set_yogi_verbosity(verbosity::INFO);
    Logger::set_max_yogi_verbosity(verbosity::ERROR);
    EXPECT_EQ(verbosity::ERROR, Logger().effective_yogi_verbosity());
    Logger::set_max_yogi_verbosity(verbosity::DEBUG);
    EXPECT_EQ(verbosity::INFO, Logger().effective_yogi_verbosity());

    Logger().set_stdout_verbosity(verbosity::INFO);
    Logger::set_max_stdout_verbosity(verbosity::ERROR);
    EXPECT_EQ(verbosity::ERROR, Logger().effective_stdout_verbosity());
    Logger::set_max_stdout_verbosity(verbosity::DEBUG);
    EXPECT_EQ(verbosity::INFO, Logger().effective_stdout_verbosity());

    Logger().set_yogi_verbosity(verbosity::TRACE);
    EXPECT_EQ(verbosity::DEBUG, Logger().max_effective_verbosity());
}

TEST_F(LoggingTest, ProcessInterface)
{
    Logger::set_colourised_stdout(false);

    const char* argv[] = {
        "executable",
        "-j",
        R"(
            {
                "logging": {
                    "stdout": {
                        "colourised": true,
                        "max-verbosity": "DEBUG",
                        "component-verbosity": {
                            "App"  : "ERROR",
                            "Yogi" : "FATAL",
                            "test" : "TRACE"
                        }
                    },
                    "yogi": {
                        "max-verbosity": "INFO",
                        "component-verbosity": {
                            "App"  : "DEBUG",
                            "Yogi" : "INFO",
                            "test" : "WARNING"
                        }
                    }
                }
            }
        )"
    };
    ProcessInterface pi(3, argv);

    // check initial configuration
    EXPECT_TRUE(Logger::colourised_stdout());
    EXPECT_EQ(verbosity::DEBUG, Logger::max_stdout_verbosity());
    EXPECT_EQ(verbosity::ERROR, Logger().stdout_verbosity());
    EXPECT_EQ(verbosity::ERROR, Logger("App").stdout_verbosity());
    EXPECT_EQ(verbosity::FATAL, Logger("Yogi").stdout_verbosity());
    EXPECT_EQ(verbosity::TRACE, Logger("test").stdout_verbosity());

    EXPECT_EQ(verbosity::INFO, Logger::max_yogi_verbosity());
    EXPECT_EQ(verbosity::DEBUG, Logger().yogi_verbosity());
    EXPECT_EQ(verbosity::DEBUG, Logger("App").yogi_verbosity());
    EXPECT_EQ(verbosity::INFO, Logger("Yogi").yogi_verbosity());
    EXPECT_EQ(verbosity::WARNING, Logger("test").yogi_verbosity());

    // check logging over YOGI
    Leaf leaf(pi.scheduler());
    LocalConnection conn(leaf, pi.leaf());
    ConsumerTerminal<internal::yogi_000009cd> logTerminal(leaf, "/Process/Log");
    while (logTerminal.get_binding_state() == RELEASED);

    auto msg = monitor_log(logTerminal, [&] {
        YOGI_LOG_FATAL("Hello");
    });
    EXPECT_EQ("Hello", msg.value().first());

    std::stringstream ss(msg.value().second());
    pt::ptree json;
    pt::read_json(ss, json);
    EXPECT_EQ("FATAL", json.get<std::string>("severity"));
    EXPECT_TRUE(json.get<std::string>("thread").front() == 'T');
    EXPECT_FALSE(json.get<std::string>("file").empty());
    EXPECT_GT(json.get<int>("line"), 0);
    EXPECT_FALSE(json.get<std::string>("function").empty());
    EXPECT_FALSE(json.get<std::string>("component").empty());

    // check changing verbosity levels at runtime
    Logger logger("My Component");

    CachedSlaveTerminal<internal::yogi_00004004> maxVerbosityTerminal(leaf, "/Process/YOGI Log Verbosity/Max Verbosity");
    while (maxVerbosityTerminal.get_binding_state() == RELEASED);
    while (maxVerbosityTerminal.get_subscription_state() == UNSUBSCRIBED);

    EXPECT_EQ(static_cast<std::uint8_t>(verbosity::INFO), monitor_verbosity(maxVerbosityTerminal, [&] {
        auto msg = maxVerbosityTerminal.make_message();
        msg.set_value(static_cast<std::uint8_t>(verbosity::INFO));
        maxVerbosityTerminal.publish(msg);
    }));

    CachedSlaveTerminal<internal::yogi_00004004> verbosityTerminal(leaf, "/Process/YOGI Log Verbosity/Components/My Component");
    while (verbosityTerminal.get_binding_state() == RELEASED);
    while (verbosityTerminal.get_subscription_state() == UNSUBSCRIBED);

    EXPECT_EQ(static_cast<std::uint8_t>(verbosity::DEBUG), monitor_verbosity(verbosityTerminal, [&] {
        auto msg = verbosityTerminal.make_message();
        msg.set_value(static_cast<std::uint8_t>(verbosity::DEBUG));
        verbosityTerminal.publish(msg);
    }));

    EXPECT_EQ("Go", monitor_log(logTerminal, [&] {
        YOGI_LOG_INFO(logger, "Go");
    }).value().first());

    EXPECT_EQ(static_cast<std::uint8_t>(verbosity::WARNING), monitor_verbosity(verbosityTerminal, [&] {
        auto msg = verbosityTerminal.make_message();
        msg.set_value(static_cast<std::uint8_t>(verbosity::WARNING));
        verbosityTerminal.publish(msg);
    }));

    EXPECT_EQ("Vegan", monitor_log(logTerminal, [&] {
        YOGI_LOG_INFO(logger, "Welcome");
        YOGI_LOG_WARNING(logger, "Vegan");
    }).value().first());

    EXPECT_EQ(static_cast<std::uint8_t>(verbosity::ERROR), monitor_verbosity(maxVerbosityTerminal, [&] {
        auto msg = maxVerbosityTerminal.make_message();
        msg.set_value(static_cast<std::uint8_t>(verbosity::ERROR));
        maxVerbosityTerminal.publish(msg);
    }));

    EXPECT_EQ("Mate", monitor_log(logTerminal, [&] {
        YOGI_LOG_WARNING(logger, "Dude");
        YOGI_LOG_ERROR(logger, "Mate");
    }).value().first());
}
