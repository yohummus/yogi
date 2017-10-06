#include <gtest/gtest.h>

#include "../yogi/object.hpp"

#include <yogi_core.h>


class TestObject : public yogi::Object
{
public:
    TestObject(void* handle)
    : yogi::Object(handle)
    {
    }

    virtual ~TestObject() override
    {
        this->_destroy();
    }
};

class TestLeaf : public yogi::Object
{
public:
    TestLeaf()
    : yogi::Object(YOGI_CreateLeaf, nullptr)
    {
    }

    virtual ~TestLeaf() override
    {
        this->_destroy();
    }
};

struct ObjectTest : public testing::Test
{
};

TEST_F(ObjectTest, ThrowOnFailure)
{
    EXPECT_THROW(TestLeaf(), yogi::Failure);
}

TEST_F(ObjectTest, Destruction)
{
    void* handle;
    YOGI_CreateScheduler(&handle);
    {{
    TestObject scheduler(handle);
    }}
    EXPECT_NE(YOGI_Destroy(handle), YOGI_OK);
}

TEST_F(ObjectTest, ClassName)
{
    void* handle;
    YOGI_CreateScheduler(&handle);
    TestObject scheduler(handle);
    EXPECT_FALSE(scheduler.class_name().empty());
}

TEST_F(ObjectTest, ToString)
{
    void* handle;
    YOGI_CreateScheduler(&handle);
    TestObject scheduler(handle);
    EXPECT_FALSE(scheduler.to_string().empty());

    std::stringstream ss;
    ss << scheduler;
    EXPECT_FALSE(ss.str().empty());
}
