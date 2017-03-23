#include <gtest/gtest.h>

#include "../yogi/scheduler.hpp"
using namespace yogi;


struct SchedulerTest : public testing::Test
{
};

TEST_F(SchedulerTest, SetThreadPoolSize)
{
    Scheduler scheduler;
    EXPECT_NO_THROW(scheduler.set_thread_pool_size(3));
    EXPECT_THROW(scheduler.set_thread_pool_size(999999), Failure);
}
