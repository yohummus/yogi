#include "../../src/base/ObjectRegister.hpp"
using namespace chirp::base;

#include <gmock/gmock.h>


struct ObjectRegisterTest: public testing::Test
{
    ObjectRegister<int> uut;
};

TEST_F(ObjectRegisterTest, BasicBehavior)
{
    auto id1 = uut.insert(10);
    auto id2 = uut.insert(20);
    auto id3 = uut.insert(30);

    EXPECT_EQ(10, uut[id1]);
    EXPECT_EQ(20, uut[id2]);
    EXPECT_EQ(30, uut[id3]);

    uut.erase(id2);
    EXPECT_EQ(10, uut[id1]);
    EXPECT_EQ(30, uut[id3]);

    auto id4 = uut.insert(40);
    EXPECT_EQ(id4, id2);
    EXPECT_EQ(10, uut[id1]);
    EXPECT_EQ(30, uut[id3]);
    EXPECT_EQ(40, uut[id4]);
}
