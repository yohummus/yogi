#ifndef YOGI_TESTS_MOCKS_DEAFMUTEBINDINGMOCK_HPP
#define YOGI_TESTS_MOCKS_DEAFMUTEBINDINGMOCK_HPP

#include "../../src/core/BindingT.hpp"
using namespace yogi;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

template <typename TLeafLogic>
struct BindingMock : public core::BindingT<TLeafLogic>
{
    typedef core::BindingT<TLeafLogic> super;
    typedef typename super::state_t state_t;
    typedef typename super::handler_fn handler_fn;

    BindingMock(interfaces::ITerminal& terminal,
        base::Identifier::name_type targets, bool hiddenTargets = false)
        : super(terminal, targets, hiddenTargets)
    {
        EXPECT_CALL(*this, publish_state(_))
            .Times(AnyNumber());
    }

    MOCK_CONST_METHOD0_T(state, state_t ());
    MOCK_METHOD1_T(async_get_state, void (handler_fn));
    MOCK_METHOD1_T(async_await_state_change, void (handler_fn));
    MOCK_METHOD0(cancel_await_state_change, void ());
    MOCK_METHOD1_T(publish_state, void (state_t));
};

} // namespace mocks

#endif // YOGI_TESTS_MOCKS_DEAFMUTEBINDINGMOCK_HPP
