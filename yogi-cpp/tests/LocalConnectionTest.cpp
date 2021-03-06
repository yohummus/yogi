#include <gtest/gtest.h>

#include "../yogi/api.hpp"
#include "../yogi/connection.hpp"
#include "../yogi/scheduler.hpp"
#include "../yogi/leaf.hpp"
using namespace yogi;

#include <atomic>
#include <thread>


struct LocalConnectionTest : public testing::Test
{
    Scheduler scheduler;
    Leaf      leafA;
    Leaf      leafB;
    Leaf      leafC;

    LocalConnectionTest()
    : leafA(scheduler)
    , leafB(scheduler)
    , leafC(scheduler)
    {
    }
};

TEST_F(LocalConnectionTest, LocalConnection)
{
    LocalConnection conn(leafA, leafB);
    EXPECT_FALSE(conn.description().empty());
    EXPECT_EQ(get_version(), conn.remote_version());
    EXPECT_FALSE(conn.remote_identification());
}
