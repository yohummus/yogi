#include "../../src/api/ExceptionT.hpp"
using namespace chirp::api;

#include <gmock/gmock.h>


struct ExceptionTest : public testing::Test
{
};


TEST_F(ExceptionTest, GetDescription)
{
    EXPECT_STREQ(ExceptionT<CHIRP_OK>{}.what(),
        Exception::get_description(CHIRP_OK));
    EXPECT_STREQ("Unknown internal error occurred", Exception::get_description(
        CHIRP_ERR_UNKNOWN));
    EXPECT_STREQ("Unknown error code",
        Exception::get_description(23452));
}
