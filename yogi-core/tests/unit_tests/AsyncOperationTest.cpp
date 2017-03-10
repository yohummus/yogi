#include "../../src/base/AsyncOperation.hpp"
using namespace chirp;

#include <gmock/gmock.h>

#include <thread>
#include <mutex>


struct AsyncOperationTest : public testing::Test
{
    typedef std::function<void (const api::Exception&, int)> handler_fn;

    base::AsyncOperation<handler_fn> op;

    std::mutex mutex;
    handler_fn fn;
    int        calls     = 0;
    int        errorCode = CHIRP_OK;
    int        arg       = 0;

    virtual void SetUp() override
    {
        fn = [&](const api::Exception& e, int x) {
            std::lock_guard<std::mutex> lock{mutex};
            ++calls;
            errorCode = e.error_code();
            arg = x;
        };
    }
};

TEST_F(AsyncOperationTest, BasicBehavior)
{
    op.arm(fn);
    EXPECT_THROW(op.arm(fn),
        api::ExceptionT<CHIRP_ERR_ASYNC_OPERATION_RUNNING>);

    op.disarm();
    EXPECT_NO_THROW(op.disarm());
    op.fire<CHIRP_OK>(123);
    EXPECT_EQ(0, calls);

    op.arm(fn);
    std::thread th([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds{1});
        op.fire<CHIRP_ERR_WRONG_OBJECT_TYPE>(500);
    });
    op.await_idle();
    EXPECT_EQ(1, calls);
    EXPECT_EQ(CHIRP_ERR_WRONG_OBJECT_TYPE, errorCode);
    EXPECT_EQ(500, arg);
    th.join();

    op.fire<CHIRP_OK>(123);
    EXPECT_EQ(1, calls);
}

TEST_F(AsyncOperationTest, NonVoidReturnType)
{
    typedef std::function<int (const api::Exception&)> handler_fn;
    base::AsyncOperation<handler_fn> op;

    op.arm([](const api::Exception&){ return 12345; });
    EXPECT_EQ(12345, op.fire<CHIRP_OK>());
}

