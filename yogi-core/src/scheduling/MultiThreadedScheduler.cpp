#include "MultiThreadedScheduler.hpp"
#include "../api/ExceptionT.hpp"

#include <algorithm>


namespace yogi {
namespace scheduling {

void MultiThreadedScheduler::thread_fn()
{
    try {
        m_ioService.run();
    }
    catch (const exit_thread_exception&) {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_terminatedThreads.push_back(std::this_thread::get_id());
        m_cv.notify_one();
    }
}

MultiThreadedScheduler::MultiThreadedScheduler()
    : m_work{m_ioService}
{
    resize_thread_pool(YOGI_DEFAULT_SCHEDULER_THREAD_POOL_SIZE);
}

MultiThreadedScheduler::~MultiThreadedScheduler()
{
    m_ioService.stop();
    for (auto& thread : m_threads) {
        thread.join();
    }
}

void MultiThreadedScheduler::resize_thread_pool(std::size_t numThreads)
{
    if (numThreads < 1 || numThreads > YOGI_MAX_SCHEDULER_THREAD_POOL_SIZE) {
        throw api::ExceptionT<YOGI_ERR_INVALID_PARAM>{};
    }

    std::unique_lock<std::mutex> lock{m_mutex};

    // remove threads
    if (numThreads < m_threads.size()) {
        // terminate some threads
        for (auto i = m_threads.size(); i > numThreads; --i) {
            m_ioService.post([]{ throw exit_thread_exception(); });
        }

        // wait for the threads to terminate
        auto diff = m_threads.size() - numThreads;
        m_cv.wait(lock, [&]{ return m_terminatedThreads.size() == diff; });

        // join the terminated threads
        for (auto& id : m_terminatedThreads) {
            auto it = std::find_if(m_threads.begin(), m_threads.end(),
                [&](const std::thread& th){ return th.get_id() == id; });
            it->join();
            m_threads.erase(it);
        }

        m_terminatedThreads.clear();
    }
    // create new threads
    else if (numThreads > m_threads.size()) {
        for (auto i = m_threads.size(); i < numThreads; ++i) {
            m_threads.push_back(std::thread(&MultiThreadedScheduler::thread_fn,
                this));
        }
    }
}

boost::asio::io_service& MultiThreadedScheduler::io_service()
{
    return m_ioService;
}

} // namespace scheduling
} // namespace yogi
