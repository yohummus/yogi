#ifndef CHIRP_TESTS_MOCKS_PUBLISHSUBSCRIBETERMINALMOCK_HPP
#define CHIRP_TESTS_MOCKS_PUBLISHSUBSCRIBETERMINALMOCK_HPP

#include "../../src/core/publish_subscribe/Terminal.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct PublishSubscribeTerminalMock
    : public core::publish_subscribe::Terminal<>
{
    PublishSubscribeTerminalMock(core::Leaf& leaf,
        base::Identifier identifier)
        : core::publish_subscribe::Terminal<>(leaf, identifier)
    {
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_PUBLISHSUBSCRIBETERMINALMOCK_HPP
