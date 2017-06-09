#ifndef YOGI_TESTS_MOCKS_LEAFMOCK_HPP
#define YOGI_TESTS_MOCKS_LEAFMOCK_HPP

#include "SchedulerMock.hpp"
#include "../../src/core/Leaf.hpp"

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct LeafMock : public core::Leaf
{
    LeafMock(std::shared_ptr<SchedulerMock> scheduler_
        = std::make_shared<SchedulerMock>())
        : core::Leaf(*scheduler_)
    {
    }

    virtual std::pair<base::Id, std::unique_lock<std::recursive_mutex>>
        sg_scatter(core::scatter_gather::Terminal<>& terminal,
            base::Buffer&& data) override
    {
        static std::recursive_mutex m;
        return std::make_pair(scatter_(terminal, data),
            std::unique_lock<std::recursive_mutex>(m));
    }

    MOCK_METHOD2(scatter_, base::Id (core::scatter_gather::Terminal<>&,
        base::Buffer&));

    virtual std::unique_lock<std::recursive_mutex> sg_cancel_scatter(
        core::scatter_gather::Terminal<>& terminal, base::Id id) override
    {
        cancel_scatter_(terminal, id);

        static std::recursive_mutex m;
        return std::unique_lock<std::recursive_mutex>(m);
    }

    MOCK_METHOD2(cancel_scatter_, void (core::scatter_gather::Terminal<>&,
        base::Id));

    virtual void sg_respond_to_scattered_message(
        core::scatter_gather::Terminal<>& terminal,
        base::Id operationId,
        core::scatter_gather::gather_flags flags, bool acquireMutex,
        base::Buffer&& data) override
    {
        respond_to_scattered_message_(terminal, operationId, flags,
            acquireMutex, data);
    }

    MOCK_METHOD5(respond_to_scattered_message_, void (
        core::scatter_gather::Terminal<>&, base::Id,
        core::scatter_gather::gather_flags, bool, base::Buffer&));
};

} // namespace mocks

#endif // YOGI_TESTS_MOCKS_LEAFMOCK_HPP
