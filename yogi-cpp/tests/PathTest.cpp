#include <gtest/gtest.h>

#include "../yogi/path.hpp"
using namespace yogi;


struct PathTest : public testing::Test
{
};

TEST_F(PathTest, BadPath)
{
    EXPECT_THROW(Path("//"),     BadPath);
    EXPECT_THROW(Path("/tmp//"), BadPath);
    EXPECT_THROW(Path("tmp//"),  BadPath);
    EXPECT_THROW(Path("a//b"),   BadPath);
}

TEST_F(PathTest, Size)
{
    Path path("/Test");
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(5u, path.size());
    path.clear();
    EXPECT_TRUE(path.empty());
    EXPECT_EQ(0u, path.size());
}

TEST_F(PathTest, Empty)
{
    Path path("/Test");
    EXPECT_FALSE(path.empty());
    path.clear();
    EXPECT_TRUE(path.empty());
}

TEST_F(PathTest, ToString)
{
    Path path("/Test");
    EXPECT_EQ("/Test", path.to_string());
}

TEST_F(PathTest, Operators)
{
    EXPECT_EQ(Path("/Test/tmp"), Path("/Test") / Path("tmp"));
    EXPECT_EQ(Path("/Test/tmp"), Path("/Test") / "tmp");
    EXPECT_THROW(Path("/Test") / Path("/tmp"), BadPath);
}

TEST_F(PathTest, IsRoot)
{
    Path path1("/");
    EXPECT_TRUE(path1.is_root());

    Path path2("/Test");
    EXPECT_FALSE(path2.is_root());

    Path path3("");
    EXPECT_FALSE(path3.is_root());
}

TEST_F(PathTest, IsAbsolute)
{
    Path path1("/Test");
    EXPECT_TRUE(path1.is_absolute());

    Path path2("Test");
    EXPECT_FALSE(path2.is_absolute());
}
