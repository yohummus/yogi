#ifndef CHIRP_TESTS_MOCKS_CACHEDPUBLISHSUBSCRIBETERMINALMOCK_HPP
#define CHIRP_TESTS_MOCKS_CACHEDPUBLISHSUBSCRIBETERMINALMOCK_HPP

#include "../../src/core/cached_publish_subscribe/Terminal.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct CachedPublishSubscribeTerminalMock
    : public core::cached_publish_subscribe::Terminal<>
{
    CachedPublishSubscribeTerminalMock(core::Leaf& leaf,
        base::Identifier identifier)
        : core::cached_publish_subscribe::Terminal<>(leaf, identifier)
    {
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_CACHEDPUBLISHSUBSCRIBETERMINALMOCK_HPP
