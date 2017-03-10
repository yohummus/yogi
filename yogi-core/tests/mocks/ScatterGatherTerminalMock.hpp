#ifndef CHIRP_TESTS_MOCKS_SCATTERGATHERTERMINALMOCK_HPP
#define CHIRP_TESTS_MOCKS_SCATTERGATHERTERMINALMOCK_HPP

#include "../../src/core/scatter_gather/Terminal.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct ScatterGatherTerminalMock
    : public core::scatter_gather::Terminal<>
{
    MOCK_METHOD2(on_scattered_message_received_, void (base::Id,
        base::Buffer&));

    virtual void on_scattered_message_received(base::Id operationId,
        base::Buffer&& data) override
    {
        on_scattered_message_received_(operationId, data);
    }

    MOCK_METHOD3(on_gathered_message_received_, bool (base::Id,
        core::scatter_gather::gather_flags flags,
        base::Buffer& data));

    virtual bool on_gathered_message_received(base::Id operationId,
        core::scatter_gather::gather_flags flags,
        base::Buffer&& data) override
    {
        return on_gathered_message_received_(operationId, flags, data);
    }

    ScatterGatherTerminalMock(core::Leaf& leaf,
        base::Identifier identifier)
        : core::scatter_gather::Terminal<>(leaf, identifier)
    {
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_SCATTERGATHERTERMINALMOCK_HPP
