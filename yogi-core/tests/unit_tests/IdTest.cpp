#include "../../src/base/Id.hpp"
using namespace chirp::base;

#include <gmock/gmock.h>


struct IdTest : public testing::Test
{
};

TEST_F(IdTest, InvalidNumber)
{
    EXPECT_EQ(0, Id::invalid_number());
}

TEST_F(IdTest, DefaultCtor)
{
    EXPECT_EQ(Id::invalid_number(), Id{}.number());
}

TEST_F(IdTest, Number)
{
    EXPECT_EQ(123, Id{123}.number());
}

TEST_F(IdTest, Valid)
{
    EXPECT_TRUE(Id{123}.valid());
    EXPECT_FALSE(Id{}.valid());
}

TEST_F(IdTest, Operators)
{
    EXPECT_TRUE((bool)Id{123});
    EXPECT_FALSE((bool)Id{});

    EXPECT_TRUE(Id{1}  == Id{1});
    EXPECT_FALSE(Id{1} == Id{2});

    EXPECT_TRUE(Id{1}  != Id{2});
    EXPECT_FALSE(Id{1} != Id{1});
}
