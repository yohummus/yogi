#include <gtest/gtest.h>

#include "../yogi/result.hpp"

#include <yogi_core.h>


struct ResultTest : public testing::Test
{
};

TEST_F(ResultTest, Operators)
{
    yogi::Result a(YOGI_OK);
    yogi::Result b(YOGI_ERR_UNKNOWN);
    yogi::Result c(YOGI_ERR_BAD_ALLOCATION);
    yogi::Success ok;
    yogi::Failure nok(YOGI_ERR_UNKNOWN);

    EXPECT_TRUE(!!a);
    EXPECT_FALSE(b);
    EXPECT_FALSE(c);
    EXPECT_TRUE(!!ok);
    EXPECT_FALSE(nok);

    EXPECT_EQ(a, ok);
    EXPECT_NE(a, b);
    EXPECT_EQ(b, nok);
    EXPECT_NE(b, c);
}

TEST_F(ResultTest, Value)
{
    yogi::Result a(YOGI_OK);
    EXPECT_EQ(YOGI_OK, a.value());

    yogi::Result b(YOGI_ERR_UNKNOWN);
    EXPECT_EQ(YOGI_ERR_UNKNOWN, b.value());
}

TEST_F(ResultTest, ToString)
{
    yogi::Result a(YOGI_OK);
    EXPECT_TRUE(a.to_string().find(YOGI_GetErrorString(YOGI_OK)) != std::string::npos);

    yogi::Result b(YOGI_ERR_UNKNOWN);
    EXPECT_TRUE(b.to_string().find(YOGI_GetErrorString(YOGI_ERR_UNKNOWN)) != std::string::npos);

    std::stringstream ss;
    ss << b;
    EXPECT_TRUE(ss.str().find(YOGI_GetErrorString(YOGI_ERR_UNKNOWN)) != std::string::npos);
}
