#include <gtest/gtest.h>

#include "../yogi/configuration.hpp"
#include "../yogi/internal/glob.hpp"
using namespace yogi;

#include <boost/filesystem.hpp>

#include <fstream>
#include <algorithm>


struct GlobTest : public testing::Test
{
    boost::filesystem::path oldWorkingDirectory;
    boost::filesystem::path tempPath;

    virtual void SetUp() override
    {
        tempPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
        boost::filesystem::create_directory(tempPath);
        oldWorkingDirectory = boost::filesystem::current_path();
        boost::filesystem::current_path(tempPath);

        boost::filesystem::create_directory("test");
        std::ofstream("test/a.json");
        std::ofstream("test/b.json");
        boost::filesystem::create_directory("test/config");
        std::ofstream("test/config/common.json");
        std::ofstream("test/config/utils.json");
        std::ofstream("test/config/utils.ini");
        boost::filesystem::create_directory("test/config/old");
        std::ofstream("test/config/old/one.json");
        std::ofstream("test/config/old/two.json");
        std::ofstream("test/config/old/three.ini");
        boost::filesystem::create_directory("test/config/new");
        std::ofstream("test/config/new/all.json");
        boost::filesystem::create_directory("test/stuff");
        std::ofstream("test/stuff/more.ini");
        std::ofstream("test/stuff/even_more.json");

        boost::filesystem::current_path("test/config");
    }

    virtual void TearDown() override
    {
        boost::filesystem::current_path(oldWorkingDirectory);
        boost::filesystem::remove_all(tempPath);
    }

    void check(std::vector<std::string> patterns, std::vector<std::string> expectedFilenames)
    {
        auto filenames = internal::glob(patterns);
        EXPECT_EQ(expectedFilenames.size(), filenames.size());

        for (auto& filename : expectedFilenames) {
            bool found = std::find_if(filenames.begin(), filenames.end(), [&](auto& s) {
                return boost::filesystem::equivalent(filename, s);
            }) != filenames.end();
            EXPECT_TRUE(found) << "Filename '" << filename << "' expected but not found by glob()";
        }

        for (auto& filename : filenames) {
            bool found = std::find_if(expectedFilenames.begin(), expectedFilenames.end(), [&](auto& s) {
                return boost::filesystem::equivalent(filename, s);
            }) != expectedFilenames.end();
            EXPECT_TRUE(found) << "Filename '" << filename << "' unexpectedly found by glob()";
        }
    }
};

TEST_F(GlobTest, ExplicitFilenames)
{
    std::vector<std::string> filenames = {
        "old/one.json",
        "./old/three.ini",
        "../config/common.json",
        "../stuff/more.ini",
        ".././../test/./stuff/../config/utils.json",
        "../../test/./stuff/../config/utils.ini"
    };

    check(filenames, filenames);
}

TEST_F(GlobTest, FilesInWorkingDirectory)
{
    check({
        "*"
    }, {
        "common.json",
        "utils.json",
        "utils.ini"
    });

    check({
        "*.js*"
    }, {
        "common.json",
        "utils.json"
    });
}

TEST_F(GlobTest, FilesInParentDirectory)
{
    check({
        "../*"
    }, {
        "../a.json",
        "../b.json"
    });
}

TEST_F(GlobTest, FilesInSubDirectory)
{
    check({
        "old/*.json"
    }, {
        "old/one.json",
        "old/two.json"
    });
}

TEST_F(GlobTest, FilesInImmediateSubdirs)
{
    check({
        "*/*"
    }, {
        "old/one.json",
        "old/two.json",
        "old/three.ini",
        "new/all.json"
    });
}

TEST_F(GlobTest, AbsolutePaths)
{
    auto pwd = boost::filesystem::current_path();
    check({
        (pwd / "old/two.*").string()
    }, {
        "old/two.json"
    });
}

TEST_F(GlobTest, ResultOrder)
{
    auto filenames = internal::glob({
        "old/one.json",
        "./old/three.ini",
        "old/one.json"
    });

    EXPECT_EQ(2u, filenames.size());
    EXPECT_TRUE(boost::filesystem::equivalent(filenames[0], "old/three.ini"));
    EXPECT_TRUE(boost::filesystem::equivalent(filenames[1], "old/one.json"));
}

TEST_F(GlobTest, BadConfigurationFilePattern)
{
    EXPECT_THROW(internal::glob({
        "old/one.json",
        "./old/does_not_exist.ini"
    }), BadConfigurationFilePattern);
}
