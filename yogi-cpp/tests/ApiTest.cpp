#include <gtest/gtest.h>

#include "../yogi_cpp/api.hpp"
#include "../yogi_cpp/scheduler.hpp"

#include <yogi_core.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

struct ApiTest : public testing::Test
{
};

TEST_F(ApiTest, GetVersion)
{
    EXPECT_EQ(YOGI_GetVersion(), yogi::get_version());
}

TEST_F(ApiTest, SetLogFile)
{
    auto file = "yogi.log";
    yogi::set_log_file(file, yogi::verbosity::DEBUG);
    yogi::Scheduler scheduler;
    EXPECT_TRUE(fs::exists(file));
    EXPECT_GT(fs::file_size(file), 0u);
}
