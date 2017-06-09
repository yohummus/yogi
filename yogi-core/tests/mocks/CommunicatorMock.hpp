#ifndef YOGI_TESTS_MOCKS_COMMUNICATORMOCK_HPP
#define YOGI_TESTS_MOCKS_COMMUNICATORMOCK_HPP

#include "../../src/interfaces/ILeaf.hpp"
#include "../../src/interfaces/INode.hpp"
#include "../../src/interfaces/IScheduler.hpp"
using namespace yogi;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

template <typename TInterface>
struct CommunicatorMockT : public TInterface
{
    MOCK_METHOD0(scheduler, interfaces::IScheduler& () noexcept);
    MOCK_METHOD1(on_new_connection, void (interfaces::IConnection&));
    MOCK_METHOD1(on_connection_started, void (interfaces::IConnection&));
    MOCK_METHOD1(on_connection_destroyed, void (interfaces::IConnection&)
        noexcept);

    virtual void on_message_received(interfaces::IMessage&& msg,
        interfaces::IConnection& origin) override
    {
        on_message_received_(msg, origin);
    }

    MOCK_METHOD2(on_message_received_, void (interfaces::IMessage&,
        interfaces::IConnection&));

    CommunicatorMockT()
    {
    }

    CommunicatorMockT(interfaces::IScheduler& scheduler)
    {
        EXPECT_CALL(*this, scheduler())
            .WillRepeatedly(ReturnRef(scheduler));
    }
};

typedef CommunicatorMockT<interfaces::ICommunicator> CommunicatorMock;
typedef CommunicatorMockT<interfaces::ILeaf>         LeafCommunicatorMock;
typedef CommunicatorMockT<interfaces::INode>         NodeCommunicatorMock;

} // namespace mocks

#endif // YOGI_TESTS_MOCKS_COMMUNICATORMOCK_HPP
