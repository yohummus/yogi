#ifndef CHIRP_TESTS_MOCKS_CACHEDMASTERSLAVETERMINALMOCK_HPP
#define CHIRP_TESTS_MOCKS_CACHEDMASTERSLAVETERMINALMOCK_HPP

#include "../../src/core/cached_master_slave/Terminal.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct CachedMasterSlaveTerminalMock
    : public core::cached_master_slave::Terminal<>
{
    CachedMasterSlaveTerminalMock(core::Leaf& leaf, base::Identifier identifier)
        : core::cached_master_slave::Terminal<>(leaf, identifier)
    {
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_CACHEDMASTERSLAVETERMINALMOCK_HPP
