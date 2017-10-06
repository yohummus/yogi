#include "../../src/base/IdentifiedObjectRegister.hpp"
using namespace yogi::base;

#include <gmock/gmock.h>

#include <memory>


struct IdentifiedObjectRegisterTest: public testing::Test
{
    IdentifiedObjectRegister<std::unique_ptr<int>> uut;
};

TEST_F(IdentifiedObjectRegisterTest, Insert)
{
    auto res = uut.insert(Identifier{123u, "Moo", false},
        std::make_unique<int>(555));
    EXPECT_TRUE(res.second);
    EXPECT_EQ(Identifier(123u, "Moo", false), res.first->identifier());
    EXPECT_EQ(Id(1), res.first->id());
    EXPECT_FALSE(res.first->hidden());
    EXPECT_EQ(555, *(*res.first));
}

TEST_F(IdentifiedObjectRegisterTest, Find)
{
    auto it = uut.insert(Identifier{0u, "a", false}).first;
    EXPECT_TRUE(it == uut.find(Identifier{0u, "a", false}));
    EXPECT_TRUE(it == uut.find(Id(1)));
}

TEST_F(IdentifiedObjectRegisterTest, AccessOperator)
{
    auto it = uut.insert(Identifier{0u, "a", false}).first;
    EXPECT_EQ(&*it, &uut[(Identifier{0u, "a", false})]);
    EXPECT_EQ(&*it, &uut[Id(1)]);
}

TEST_F(IdentifiedObjectRegisterTest, Erase)
{
    auto it1 = uut.insert(Identifier{0u, "a", false}).first;
    auto it2 = uut.insert(Identifier{0u, "b", false}).first;
    EXPECT_TRUE(it2 == uut.erase(it1));
    EXPECT_TRUE(uut.end() == uut.find(Identifier{0u, "a", false}));
    EXPECT_TRUE(uut.end() == uut.find(Id(1)));
    EXPECT_TRUE(uut.end() == uut.erase(it2));
}

TEST_F(IdentifiedObjectRegisterTest, Foreach)
{
    uut.insert(Identifier{0u, "a", false});
    uut.insert(Identifier{0u, "b", false});

    int i = 0;
    for (auto& element : uut) {
        switch (i++) {
        case 0: EXPECT_EQ("a", element.identifier().name()); break;
        case 1: EXPECT_EQ("b", element.identifier().name()); break;
        default: FAIL();
        }
    }

    EXPECT_EQ(2, i);
}

TEST_F(IdentifiedObjectRegisterTest, IdAssignment)
{
    EXPECT_EQ(Id(1), uut.insert(Identifier{0u, "a", false}).first->id());
    EXPECT_EQ(Id(2), uut.insert(Identifier{0u, "b", false}).first->id());
    EXPECT_EQ(Id(3), uut.insert(Identifier{0u, "c", false}).first->id());
    uut.erase(uut.find(Id(1)));
    EXPECT_EQ(Id(1), uut.insert(Identifier{0u, "d", false}).first->id());
    EXPECT_EQ(Id(4), uut.insert(Identifier{0u, "e", false}).first->id());
}

TEST_F(IdentifiedObjectRegisterTest, Hide)
{
    auto it = uut.insert(Identifier{0u, "a", false}).first;
    uut.hide(it);
    EXPECT_TRUE(uut.end() == uut.find(Identifier{0u, "a", false}));
    EXPECT_TRUE(it == uut.find(Id(1)));
    EXPECT_TRUE(it->hidden());
}
