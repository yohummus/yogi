#ifndef CHIRP_SCHEDULING_MULTITHREADEDSCHEDULER_HPP
#define CHIRP_SCHEDULING_MULTITHREADEDSCHEDULER_HPP

#include "../config.h"
#include "../interfaces/IScheduler.hpp"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>


namespace chirp {
namespace scheduling {

/***************************************************************************//**
 * Scheduler built around a runtime-resizable thread pool
 ******************************************************************************/
class MultiThreadedScheduler : public interfaces::IScheduler
{
    struct exit_thread_exception {};

private:
    boost::asio::io_service       m_ioService;
    boost::asio::io_service::work m_work;
    std::vector<std::thread>      m_threads;
    std::vector<std::thread::id>  m_terminatedThreads;
    std::mutex                    m_mutex;
    std::condition_variable       m_cv;

private:
    void thread_fn();

public:
    MultiThreadedScheduler();
    virtual ~MultiThreadedScheduler();

    void resize_thread_pool(std::size_t numThreads);

    virtual boost::asio::io_service& io_service() override;
};

} // namespace scheduling
} // namespace chirp

#endif // CHIRP_SCHEDULING_MULTITHREADEDSCHEDULER_HPP
