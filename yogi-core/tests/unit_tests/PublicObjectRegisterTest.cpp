#include "../../src/api/PublicObjectRegister.hpp"
using namespace chirp::api;
using namespace chirp::interfaces;

#include <gmock/gmock.h>


struct Cat : public IPublicObject
{
    int age;

    Cat(int _age = 0)
        : age(_age)
    {
    }
};

struct Dog : public IPublicObject
{
};


struct PublicObjectRegisterTest : public testing::Test
{
    void* handle;

    virtual void SetUp() override
    {
        handle = PublicObjectRegister::create<Cat>(7);
    }
};


TEST_F(PublicObjectRegisterTest, Get)
{
    auto& obj = PublicObjectRegister::get<Cat>(handle);
    EXPECT_EQ(7, obj.age);
}

TEST_F(PublicObjectRegisterTest, GetSafe)
{
    auto& obj = PublicObjectRegister::get_s<Cat>(handle);
    EXPECT_EQ(7, obj.age);

    EXPECT_THROW(PublicObjectRegister::get_s<Cat>((void*)555),
        ExceptionT<CHIRP_ERR_INVALID_HANDLE>);

    EXPECT_THROW(PublicObjectRegister::get_s<Dog>(handle),
        ExceptionT<CHIRP_ERR_WRONG_OBJECT_TYPE>);
}

TEST_F(PublicObjectRegisterTest, Create)
{
    void* handle = PublicObjectRegister::create<Cat>(3);
    EXPECT_NE(nullptr, handle);
}

TEST_F(PublicObjectRegisterTest, Destroy)
{
    EXPECT_THROW(PublicObjectRegister::destroy((void*)555),
        ExceptionT<CHIRP_ERR_INVALID_HANDLE>);

    auto& obj = PublicObjectRegister::get<Cat>(handle);
    auto  ref = obj.shared_from_this();
    EXPECT_THROW(PublicObjectRegister::destroy(handle),
        ExceptionT<CHIRP_ERR_OBJECT_STILL_USED>);
    ref.reset();

    EXPECT_NO_THROW(PublicObjectRegister::destroy(handle));
}

TEST_F(PublicObjectRegisterTest, Clear)
{
    EXPECT_NO_THROW(PublicObjectRegister::clear());

    EXPECT_THROW(PublicObjectRegister::get_s<Cat>(handle),
        ExceptionT<CHIRP_ERR_INVALID_HANDLE>);
}
