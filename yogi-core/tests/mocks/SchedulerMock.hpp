#ifndef CHIRP_TESTS_MOCKS_SCHEDULERMOCK_HPP
#define CHIRP_TESTS_MOCKS_SCHEDULERMOCK_HPP

#include "../../src/interfaces/IScheduler.hpp"
using namespace chirp;

#include <gmock/gmock.h>
using namespace testing;


namespace mocks {

struct SchedulerMock : public interfaces::IScheduler
{
    MOCK_METHOD0(io_service, boost::asio::io_service& () noexcept);

    SchedulerMock()
    {
    }

    SchedulerMock(boost::asio::io_service& ioService)
    {
        EXPECT_CALL(*this, io_service())
            .WillRepeatedly(ReturnRef(ioService));
    }
};

} // namespace mocks

#endif // CHIRP_TESTS_MOCKS_SCHEDULERMOCK_HPP
