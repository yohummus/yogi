#include "../../src/api/ExceptionT.hpp"
using namespace yogi::api;

#include <gmock/gmock.h>


struct ExceptionTest : public testing::Test
{
};


TEST_F(ExceptionTest, GetDescription)
{
    EXPECT_STREQ(ExceptionT<YOGI_OK>{}.what(),
        Exception::get_description(YOGI_OK));
    EXPECT_STREQ("Unknown internal error occurred", Exception::get_description(
        YOGI_ERR_UNKNOWN));
    EXPECT_STREQ("Unknown error code",
        Exception::get_description(23452));
}
