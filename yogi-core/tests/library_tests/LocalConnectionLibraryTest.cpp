#include "../helpers/library_helpers.hpp"
#include "../helpers/CallbackHandler.hpp"
#include "../../src/config.h"

#include <gmock/gmock.h>


struct LocalConnectionLibraryTest : public testing::Test
{
    void* scheduler;
    void* leafA;
    void* leafB;
    void* conn;

    virtual void SetUp() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Initialise());

        scheduler = helpers::make_scheduler();
        leafA     = helpers::make_leaf(scheduler);
        leafB     = helpers::make_leaf(scheduler);
        conn      = helpers::make_connection(leafA, leafB);
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(YOGI_OK, YOGI_Shutdown());
    }
};


TEST_F(LocalConnectionLibraryTest, ConnectionInformation)
{
    char buffer[100];

    int res = YOGI_GetConnectionDescription(conn, buffer, sizeof(buffer));
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_STREQ("Local Connection", buffer);

    res = YOGI_GetRemoteVersion(conn, buffer, sizeof(buffer));
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_STREQ(YOGI_GetVersion(), buffer);

    unsigned n;
    res = YOGI_GetRemoteIdentification(conn, buffer, sizeof(buffer), &n);
    EXPECT_EQ(YOGI_OK, res);
    EXPECT_EQ(0, n);
}
