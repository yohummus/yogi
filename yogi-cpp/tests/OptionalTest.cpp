#include <gtest/gtest.h>

#include "../yogi/optional.hpp"
using namespace yogi;


struct OptionalTest : public testing::Test
{
};

TEST_F(OptionalTest, Construction)
{
    Optional<std::string> a;
    EXPECT_FALSE(a.has_value());

    std::string str = "test";
    Optional<std::string> b(str);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ("test", b.value());

    Optional<std::string> c(std::move(str));
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ("test", c.value());

    Optional<std::string> d(b);
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ("test", d.value());

    Optional<std::string> e(std::move(b));
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ("test", e.value());

    Optional<std::string> f(none);
    EXPECT_FALSE(f.has_value());
}

TEST_F(OptionalTest, HasValue)
{
    Optional<std::string> a("test");
    EXPECT_TRUE(a.has_value());
    EXPECT_TRUE(!!a);

    Optional<std::string> b;
    EXPECT_FALSE(b.has_value());
    EXPECT_FALSE(b);
}

TEST_F(OptionalTest, Assignment)
{
    Optional<std::string> a;
    Optional<std::string> b("test");
    a = b;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ("test", a.value());

    Optional<std::string> c;
    c = std::move(b);
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ("test", c.value());

    Optional<std::string> d(b);
    d = none;
    EXPECT_FALSE(d.has_value());
}

TEST_F(OptionalTest, ValueAccess)
{
    Optional<std::string> a("test");
    EXPECT_EQ("test", a.value());
    EXPECT_EQ("test", *a);
    EXPECT_EQ(4u, a->size());
    *a = "hello";
    EXPECT_EQ("hello", a.value());
    a->clear();
    EXPECT_TRUE(a.value().empty());
}

TEST_F(OptionalTest, ValueOr)
{
    Optional<std::string> a("test");
    Optional<std::string> b;

    EXPECT_EQ("test", a.value_or("hello"));
    EXPECT_EQ("hello", b.value_or("hello"));
}

TEST_F(OptionalTest, Reset)
{
    Optional<std::string> a("test");
    a.reset();
    EXPECT_FALSE(a.has_value());
}

TEST_F(OptionalTest, Comparison)
{
    Optional<std::string> a("test");
    Optional<std::string> b("test");
    Optional<std::string> c("hello");
    Optional<std::string> d;

    EXPECT_TRUE(a == a);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a != d);
    EXPECT_TRUE(d == d);

    EXPECT_TRUE(a == "test");
    EXPECT_FALSE(a == "");
    EXPECT_FALSE(d == "");
}
