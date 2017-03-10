#ifndef CHIRP_TESTS_MOCKS_MASTERSLAVETERMINALMOCK_HPP
#define CHIRP_TESTS_MOCKS_MASTERSLAVETERMINALMOCK_HPP

#include "../../src/core/master_slave/Terminal.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct MasterSlaveTerminalMock
    : public core::master_slave::Terminal<>
{
    MasterSlaveTerminalMock(core::Leaf& leaf, base::Identifier identifier)
        : core::master_slave::Terminal<>(leaf, identifier)
    {
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_MASTERSLAVETERMINALMOCK_HPP
