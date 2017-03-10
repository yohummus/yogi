#ifndef CHIRP_TESTS_MOCKS_DEAFMUTETERMINALMOCK_HPP
#define CHIRP_TESTS_MOCKS_DEAFMUTETERMINALMOCK_HPP

#include "../../src/core/deaf_mute/Terminal.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct DeafMuteTerminalMock : public core::deaf_mute::Terminal<>
{
    DeafMuteTerminalMock(core::Leaf& leaf, base::Identifier identifier)
        : core::deaf_mute::Terminal<>(leaf, identifier)
    {
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_DEAFMUTETERMINALMOCK_HPP
