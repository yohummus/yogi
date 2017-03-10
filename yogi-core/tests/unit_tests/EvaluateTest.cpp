#include "../../src/api/evaluate.hpp"
#include "../../src/api/ExceptionT.hpp"
using namespace chirp::api;

#include <gmock/gmock.h>


struct EvaluateTest : public testing::Test
{
};


TEST_F(EvaluateTest, VoidFunction)
{
    EXPECT_EQ(CHIRP_OK, evaluate(
        []{}));
    EXPECT_EQ(CHIRP_ERR_BAD_ALLOCATION, evaluate(
        []{throw std::bad_alloc{};}));
    EXPECT_EQ(CHIRP_ERR_INVALID_HANDLE, evaluate(
        []{throw ExceptionT<CHIRP_ERR_INVALID_HANDLE>{};}));
    EXPECT_EQ(CHIRP_ERR_UNKNOWN, evaluate(
        []{throw 3;}));
}

TEST_F(EvaluateTest, NonVoidFunction)
{
    EXPECT_EQ(100, evaluate(
        []{return 100;}));
    EXPECT_EQ(CHIRP_ERR_BAD_ALLOCATION, evaluate(
        []() -> int {throw std::bad_alloc{};}));
    EXPECT_EQ(CHIRP_ERR_INVALID_HANDLE, evaluate(
        []() -> int {throw ExceptionT<CHIRP_ERR_INVALID_HANDLE>{};}));
    EXPECT_EQ(CHIRP_ERR_UNKNOWN, evaluate(
        []() -> int {throw 3;}));
}
