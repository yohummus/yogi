#ifndef YOGI_TESTS_MOCKS_CONNECTIONMOCK_HPP
#define YOGI_TESTS_MOCKS_CONNECTIONMOCK_HPP

#include "../../src/interfaces/IConnection.hpp"
using namespace yogi;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct ConnectionMock : public interfaces::IConnection
{
    MOCK_METHOD1(send, void (const interfaces::IMessage&) noexcept);
    MOCK_CONST_METHOD0(remote_is_node, bool () noexcept);
    MOCK_CONST_METHOD0(description, const std::string& () noexcept);
	MOCK_CONST_METHOD0(remote_version, const std::string& () noexcept);
	MOCK_CONST_METHOD0(remote_identification, const std::vector<char>& ()
		noexcept);

    ConnectionMock()
    {
    }

    ConnectionMock(bool remoteIsNode)
    {
        EXPECT_CALL(*this, remote_is_node())
            .WillRepeatedly(Return(remoteIsNode));
        EXPECT_CALL(*this, description())
            .WillRepeatedly(ReturnRefOfCopy(std::string()));
    }
};

} // namespace mocks

#endif // YOGI_TESTS_MOCKS_CONNECTIONMOCK_HPP
