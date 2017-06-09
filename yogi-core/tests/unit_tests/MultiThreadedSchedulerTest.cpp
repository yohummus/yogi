#include "../../src/scheduling/MultiThreadedScheduler.hpp"
using namespace yogi::scheduling;

#include <gmock/gmock.h>

#include <mutex>
#include <atomic>
#include <thread>


struct MultiThreadedSchedulerTest : public testing::Test
{
    std::shared_ptr<MultiThreadedScheduler> uut;
    std::mutex                              mutex;

    virtual void SetUp() override
    {
        uut = std::make_shared<MultiThreadedScheduler>();
    }

    void check_parallel_tasks(size_t parallelTasks)
    {
        std::atomic<size_t> n{0};
        std::atomic<size_t> m{0};

        auto fn = [&] {
            ++n;
            {{ std::lock_guard<std::mutex> lock(mutex); }}
            ++m;
        };

        {{
            std::lock_guard<std::mutex> lock(mutex);
            for (size_t i = 0; i < parallelTasks + 1; ++i)
                uut->post(fn);

            while (n < parallelTasks)
                std::this_thread::yield();

            std::this_thread::sleep_for(std::chrono::milliseconds(5));

            EXPECT_EQ(parallelTasks, n);
        }}

        while (m != parallelTasks + 1)
            std::this_thread::yield();
    }
};

TEST_F(MultiThreadedSchedulerTest, DefaultThreadPoolSize)
{
    check_parallel_tasks(YOGI_DEFAULT_SCHEDULER_THREAD_POOL_SIZE);
}

TEST_F(MultiThreadedSchedulerTest, ResizeThreadPool)
{
    uut->resize_thread_pool(3);
    check_parallel_tasks(3);

    uut->resize_thread_pool(1);
    check_parallel_tasks(1);
}
