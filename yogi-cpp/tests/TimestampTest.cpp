#include <gtest/gtest.h>

#include "../yogi_cpp/timestamp.hpp"
using namespace yogi;

#include <thread>
#include <chrono>
#include <sstream>
using namespace std::string_literals;


struct TimestampTest : public testing::Test
{
};

TEST_F(TimestampTest, Operators)
{
    auto a = Timestamp::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto b = Timestamp::now();

    EXPECT_EQ(a, a);
    EXPECT_NE(a, b);
    EXPECT_LT(a, b);
    EXPECT_LE(a, a);
    EXPECT_LE(a, b);
    EXPECT_GT(b, a);
    EXPECT_GE(b, a);
    EXPECT_GE(a, a);
}

TEST_F(TimestampTest, ToString)
{
    auto t = Timestamp::now();

    EXPECT_NE(std::string::npos, t.to_string().find_last_of(std::to_string(t.milliseconds())));

    EXPECT_NE(std::string::npos, t.to_string(Timestamp::MILLISECONDS).find_last_of("."s + std::to_string(t.milliseconds())));
    EXPECT_NE(std::string::npos, t.to_string(Timestamp::MICROSECONDS).find_last_of("."s + std::to_string(t.microseconds())));
    EXPECT_NE(std::string::npos, t.to_string(Timestamp::NANOSECONDS) .find_last_of("."s + std::to_string(t.nanoseconds())));

    std::stringstream ss;
    ss << t;
    EXPECT_EQ(t.to_string(), ss.str());
}
