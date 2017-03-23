#include <gtest/gtest.h>

#include "../yogi/configuration.hpp"
using namespace yogi;

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;


struct Arguments
{
    int argc;
    char** argv;

    Arguments(std::initializer_list<std::string> args)
    {
        argc = static_cast<int>(args.size() + 1);
        argv = new char*[argc];

        std::string exe = "executable-name";
        argv[0] = new char[exe.size() + 1];
        std::copy(exe.begin(), exe.end(), argv[0]);
        argv[0][exe.size()] = '\0';

        auto it = args.begin();
        for (int i = 1; i < argc; ++i) {
            auto& arg = *it;
            argv[i] = new char[arg.size() + 1];
            std::copy(arg.begin(), arg.end(), argv[i]);
            argv[i][arg.size()] = '\0';
            ++it;
        }
    }

    ~Arguments()
    {
        for (int i = 0; i < argc; ++i) {
            delete[] argv[i];
        }

        delete[] argv;
    }
};

struct ConfigurationTest : public testing::Test
{
    fs::path configFilesDir = fs::path(__FILE__).parent_path().string();
};

TEST_F(ConfigurationTest, Update)
{
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg(args.argc, args.argv);
    cfg.update(R"(
        {
            "yogi": {
                "location": "/Home"
            }
        }
    )");

    EXPECT_EQ(Path("/Home"), cfg.location());

    EXPECT_THROW(cfg.update("{"), BadConfiguration);
}

TEST_F(ConfigurationTest, ConfigFile)
{
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg(args.argc, args.argv);
    EXPECT_EQ(Path("/Test"), cfg.location());
    EXPECT_TRUE(!!cfg.connection_target());
    EXPECT_EQ("localhost:12345", *cfg.connection_target());
    EXPECT_EQ(std::chrono::milliseconds(1234), cfg.connection_timeout());
    EXPECT_TRUE(!!cfg.connection_identification());
    EXPECT_EQ("Hello World", *cfg.connection_identification());
}

TEST_F(ConfigurationTest, ConfigFilePriority)
{
    Arguments args{
        (configFilesDir / "config_a.json").string(),
        (configFilesDir / "config_b.json").string()
    };

    Configuration cfg(args.argc, args.argv);
    EXPECT_EQ(Path("/Pudding"), cfg.location());
    EXPECT_FALSE(!!cfg.connection_target());
}

TEST_F(ConfigurationTest, ConfigFileRequired)
{
    Arguments args{};

    EXPECT_THROW(Configuration cfg(args.argc, args.argv, true), BadCommandLine);
}

TEST_F(ConfigurationTest, CommandLineOverride)
{
    Arguments args{
        (configFilesDir / "config_a.json").string(),
        "--connection_target=my-host:1234",
        "--connection_timeout=0.555",
        "-i", "Dude",
        "--location=/Home"
    };

    Configuration cfg(args.argc, args.argv);
    EXPECT_EQ(Path("/Home"), cfg.location());
    EXPECT_TRUE(!!cfg.connection_target());
    EXPECT_EQ("my-host:1234", *cfg.connection_target());
    EXPECT_EQ(std::chrono::milliseconds(555), cfg.connection_timeout());
    EXPECT_TRUE(!!cfg.connection_identification());
    EXPECT_EQ("Dude", *cfg.connection_identification());
}

TEST_F(ConfigurationTest, JsonOverrides)
{
    Arguments args{
        (configFilesDir / "config_a.json").string(),
        "--json={ \"my-age\": 42 }",
        "-j", "{ \"my-id\": 55 }",
        "--json={ \"yogi\": { \"location\": \"/Somewhere\" } }",
        "--location=/Home"
    };

    Configuration cfg(args.argc, args.argv);
    EXPECT_EQ(42, cfg.get<int>("my-age"));
    EXPECT_EQ(55, cfg.get<int>("my-id"));
    EXPECT_EQ(Path("/Home"), cfg.location());
}

TEST_F(ConfigurationTest, AccessConfiguration)
{
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg(args.argc, args.argv);
    EXPECT_TRUE(!!cfg.get_optional<float>("yogi.connection.timeout"));
    EXPECT_FALSE(cfg.get_optional<int>("yogi.connection.something.that.does.not.exist"));
    EXPECT_EQ("localhost:12345", cfg.get<std::string>("yogi.connection.target"));
    EXPECT_FLOAT_EQ(1.234f, cfg.get<float>("yogi.connection.timeout"));
    EXPECT_EQ(888, cfg.get<int>("yogi.connection.something.that.does.not.exist", 888));
    EXPECT_THROW(cfg.get<int>("yogi.connection.something.that.does.not.exist"), BadConfigurationPath);
    EXPECT_THROW(cfg.get<int>("yogi.connection.target"), BadConfigurationDataAccess);
    EXPECT_THROW(cfg.get<int>("yogi.connection"), BadConfigurationDataAccess);
}

TEST_F(ConfigurationTest, BadConfigurationFile)
{
    Arguments args{
        (configFilesDir / "config_c.json").string()
    };

    EXPECT_THROW(Configuration(args.argc, args.argv), BadConfiguration);
}

TEST_F(ConfigurationTest, BadCommandLine)
{
    Arguments args{
        "--hey_dude"
    };

    EXPECT_THROW(Configuration(args.argc, args.argv), BadCommandLine);
}

TEST_F(ConfigurationTest, Copyable)
{
    Arguments args{
        (configFilesDir / "config_b.json").string()
    };

    Configuration cfg2(args.argc, args.argv);

    {{
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg1(args.argc, args.argv);
    cfg2 = cfg1;

    EXPECT_EQ(cfg1.connection_target(), cfg2.connection_target());
    EXPECT_EQ(cfg1.get<float>("yogi.connection.timeout"), cfg2.get<float>("yogi.connection.timeout"));
    }}

    // check correct memory management when copying/destroying
    EXPECT_EQ("localhost:12345", cfg2.get<std::string>("yogi.connection.target"));
}

TEST_F(ConfigurationTest, Movable)
{
    Arguments args{
        (configFilesDir / "config_b.json").string()
    };

    Configuration cfg2(args.argc, args.argv);

    {{
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg1(args.argc, args.argv);
    cfg2 = std::move(cfg1);
    }}

    EXPECT_EQ("localhost:12345", *cfg2.connection_target());
    EXPECT_FLOAT_EQ(1.234f, cfg2.get<float>("yogi.connection.timeout"));
}

TEST_F(ConfigurationTest, Iterators)
{
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg(args.argc, args.argv);

    EXPECT_EQ(4u, cfg.size());
    EXPECT_FALSE(cfg.empty());
    ASSERT_EQ("yogi", cfg.begin()->first);
    auto childName = cfg.begin()->second.begin()->first;
    EXPECT_TRUE(childName == "location" || childName == "connection");

    auto connectionChild = cfg.get_child("yogi.connection");
    EXPECT_EQ(3u, connectionChild.size());
    EXPECT_FALSE(connectionChild.empty());

    int targetFound         = 0;
    int timeoutFound        = 0;
    int identificationFound = 0;
    for (auto child : connectionChild) {
        if (child.first == "target") {
            ++targetFound;
        }
        else if (child.first == "timeout") {
            ++timeoutFound;
        }
        else if (child.first == "identification") {
            ++identificationFound;
        }
    }

    EXPECT_EQ(1, targetFound);
    EXPECT_EQ(1, timeoutFound);
    EXPECT_EQ(1, identificationFound);

    EXPECT_TRUE(cfg.get_child("yogi.location").empty());
    for (auto child : cfg.get_child("yogi.location")) {
    }

    EXPECT_EQ(2u, cfg.get_child("array").size());
    for (auto child : cfg.get_child("array")) {
        auto val = child.second.get_value<int>();
        EXPECT_TRUE(val == 123 || val == 456);
    }

    EXPECT_TRUE(cfg.get_child("empty-array").empty());

    EXPECT_EQ(2u, cfg.get_child("object-array").size());
    for (auto child : cfg.get_child("object-array")) {
        auto val = child.second.begin()->second.get_value<int>();
        EXPECT_TRUE(val == 88 || val == 33);
    }
}

TEST_F(ConfigurationTest, ToString)
{
    // Does not work due to non-JSON-compliant boost::property_tree
    Arguments args{
        (configFilesDir / "config_a.json").string()
    };

    Configuration cfg(args.argc, args.argv);
    auto s = cfg.to_string();

    EXPECT_GT(s.size(), 30u);

    std::stringstream ss;
    ss << cfg;
    EXPECT_GT(ss.str().size(), 30u);
}
