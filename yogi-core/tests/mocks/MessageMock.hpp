#ifndef CHIRP_TESTS_MOCKS_MESSAGEMOCK_HPP
#define CHIRP_TESTS_MOCKS_MESSAGEMOCK_HPP

#include "../../src/interfaces/IMessage.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;

MATCHER_P(Msg, msg, msg.to_string()) {
    return arg.to_string() == msg.to_string();
}

MATCHER_P(MsgType, msg, (msg.to_string().substr(0, msg.to_string().find(' '))))
{
    return arg.type_id() == msg.type_id();
}


namespace mocks {

struct MessageMock : public interfaces::IMessage
{
    std::string str;

    MOCK_CONST_METHOD0(type_id, id_type ());
    MOCK_CONST_METHOD0(name, const char* ());
    MOCK_CONST_METHOD0(to_string, std::string ());

    // this function is not mockable, because google mock does not yet
    // support move-only return types
    virtual interfaces::message_ptr clone() const override
    {
        return std::make_unique<MessageMock>(str);
    }

    MOCK_CONST_METHOD1(serialize, void (buffer_type& buffer));
    MOCK_METHOD2(deserialize, void (const buffer_type& buffer,
        buffer_type::const_iterator));

    MessageMock()
    {
    }

    explicit MessageMock(const std::string& str_)
        : str(str_)
    {
        EXPECT_CALL(*this, to_string())
            .WillRepeatedly(Return(str));
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_MESSAGEMOCK_HPP
