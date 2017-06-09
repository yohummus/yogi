#include "../../src/api/evaluate.hpp"
#include "../../src/api/ExceptionT.hpp"
using namespace yogi::api;

#include <gmock/gmock.h>


struct EvaluateTest : public testing::Test
{
};


TEST_F(EvaluateTest, VoidFunction)
{
    EXPECT_EQ(YOGI_OK, evaluate(
        []{}));
    EXPECT_EQ(YOGI_ERR_BAD_ALLOCATION, evaluate(
        []{throw std::bad_alloc{};}));
    EXPECT_EQ(YOGI_ERR_INVALID_HANDLE, evaluate(
        []{throw ExceptionT<YOGI_ERR_INVALID_HANDLE>{};}));
    EXPECT_EQ(YOGI_ERR_UNKNOWN, evaluate(
        []{throw 3;}));
}

TEST_F(EvaluateTest, NonVoidFunction)
{
    EXPECT_EQ(100, evaluate(
        []{return 100;}));
    EXPECT_EQ(YOGI_ERR_BAD_ALLOCATION, evaluate(
        []() -> int {throw std::bad_alloc{};}));
    EXPECT_EQ(YOGI_ERR_INVALID_HANDLE, evaluate(
        []() -> int {throw ExceptionT<YOGI_ERR_INVALID_HANDLE>{};}));
    EXPECT_EQ(YOGI_ERR_UNKNOWN, evaluate(
        []() -> int {throw 3;}));
}
