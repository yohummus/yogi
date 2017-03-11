#ifndef YOGI_TESTS_SCHEDULER_HPP
#define YOGI_TESTS_SCHEDULER_HPP

#include "../../src/interfaces/IScheduler.hpp"

#include <boost/asio/io_service.hpp>

#include <thread>


namespace helpers {

class Scheduler : public yogi::interfaces::IScheduler
{
private:
    boost::asio::io_service       m_ioService;
    boost::asio::io_service::work m_work;
    std::thread                   m_thread;

public:
    Scheduler()
        : m_work{m_ioService}
    {
        m_thread = std::thread([&] {
            m_ioService.run();
        });
    }

    virtual ~Scheduler()
    {
        m_ioService.stop();
        m_thread.join();
    }

    virtual boost::asio::io_service& io_service() override
    {
        return m_ioService;
    }
};

} // namespace helpers

#endif // YOGI_TESTS_SCHEDULER_HPP
