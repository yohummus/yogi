#ifndef YOGI_BASE_LOCKFREERINGBUFFER_HPP
#define YOGI_BASE_LOCKFREERINGBUFFER_HPP

#include "../config.h"

#include <boost/asio/buffer.hpp>

#include <array>
#include <atomic>


namespace yogi {
namespace base {

/***************************************************************************//**
 * A lock-free ring buffer implementation
 *
 * This is based on the lock-free single-producer/single-consumer ringbuffer
 * implementation in boost 1.59 by Tim Blechmann.
 ******************************************************************************/
class LockFreeRingBuffer
{
    enum { MAX_SIZE = YOGI_RING_BUFFER_SIZE + 1 };

private:
    std::atomic<std::size_t> m_writeIdx;
    char m_padding[YOGI_CACHELINE_SIZE - sizeof(std::size_t)];
    std::atomic<std::size_t> m_readIdx;
    std::array<char, MAX_SIZE> m_data;

    std::size_t read_available(std::size_t writeIdx, std::size_t readIdx) const
    {
        if (writeIdx >= readIdx) {
            return writeIdx - readIdx;
        }

        return writeIdx + MAX_SIZE - readIdx;
    }

    std::size_t write_available(std::size_t writeIdx, std::size_t readIdx) const
    {
        auto n = readIdx - writeIdx - 1;
        if (writeIdx >= readIdx) {
            n += MAX_SIZE;
        }

        return n;
    }

    std::size_t next_index(std::size_t idx) const
    {
        idx += 1;
        if (idx >= MAX_SIZE) {
            idx -= MAX_SIZE;
        }

        return idx;
    }

public:
    LockFreeRingBuffer()
    {
        m_writeIdx = 0;
        m_readIdx = 0;
    }

    LockFreeRingBuffer(const LockFreeRingBuffer&) = delete;
    LockFreeRingBuffer& operator= (const LockFreeRingBuffer&) = delete;

    static std::size_t capacity()
    {
        return MAX_SIZE - 1;
    }

    bool empty()
    {
        auto wi = m_writeIdx.load(std::memory_order_relaxed);
        auto ri = m_readIdx.load(std::memory_order_relaxed);
        return wi == ri;
    }

    bool full()
    {
        auto wi = m_writeIdx.load(std::memory_order_relaxed);
        auto ri = m_readIdx.load(std::memory_order_acquire);
        return write_available(wi, ri) == 0;
    }

    char front() const
    {
        auto ri = m_readIdx.load(std::memory_order_relaxed);
        return m_data[ri];
    }

    void pop()
    {
        auto wi = m_writeIdx.load(std::memory_order_acquire);
        auto ri = m_readIdx.load(std::memory_order_relaxed);

        YOGI_ASSERT(!empty());

        auto next = next_index(ri);
        m_readIdx.store(next, std::memory_order_release);
    }

    std::size_t read(char* buffer, std::size_t maxSize)
    {
        auto wi = m_writeIdx.load(std::memory_order_acquire);
        auto ri = m_readIdx.load(std::memory_order_relaxed);

        auto avail = read_available(wi, ri);
        if (avail == 0) {
            return 0;
        }

        maxSize = std::min(maxSize, avail);

        auto newRi = ri + maxSize;
        if (newRi > MAX_SIZE) {
            auto count0 = MAX_SIZE - ri;
            auto count1 = maxSize - count0;

            std::copy(m_data.begin() + ri, m_data.begin() + MAX_SIZE, buffer);
            std::copy(m_data.begin(), m_data.begin() + count1, buffer + count0);

            newRi -= MAX_SIZE;
        }
        else {
            std::copy(m_data.begin() + ri, m_data.begin() + ri + maxSize, buffer);

            if (newRi == MAX_SIZE) {
                newRi = 0;
            }
        }

        m_readIdx.store(newRi, std::memory_order_release);
        return maxSize;
    }

    void commit_first_read_array(std::size_t n)
    {
        YOGI_ASSERT(n <= boost::asio::buffer_size(first_read_array()));

        auto wi = m_writeIdx.load(std::memory_order_acquire);
        auto ri = m_readIdx.load(std::memory_order_relaxed);

        ri += n;
        if (ri == MAX_SIZE) {
            ri = 0;
        }

        m_readIdx.store(ri, std::memory_order_release);
    }

    boost::asio::const_buffers_1 first_read_array() const
    {
        auto wi = m_writeIdx.load(std::memory_order_relaxed);
        auto ri = m_readIdx.load(std::memory_order_relaxed);

        if (wi < ri) {
            return boost::asio::buffer(m_data.data() + ri, MAX_SIZE - ri);
        }
        else {
            return boost::asio::buffer(m_data.data() + ri, wi - ri);
        }
    }

    template <typename ConstIterator>
    ConstIterator write(ConstIterator begin, ConstIterator end)
    {
        auto wi = m_writeIdx.load(std::memory_order_relaxed);
        auto ri = m_readIdx.load(std::memory_order_acquire);

        auto avail = write_available(wi, ri);
        if (avail == 0) {
            return begin;
        }

        std::size_t inputCnt = std::distance(begin, end);
        inputCnt = std::min(inputCnt, avail);

        auto newWi = wi + inputCnt;
        auto last = std::next(begin, inputCnt);

        if (newWi > MAX_SIZE) {
            auto count0 = MAX_SIZE - wi;
            auto midpoint = std::next(begin, count0);

            std::uninitialized_copy(begin, midpoint, m_data.begin() + wi);
            std::uninitialized_copy(midpoint, last, m_data.begin());

            newWi -= MAX_SIZE;
        }
        else {
            std::uninitialized_copy(begin, last, m_data.begin() + wi);

            if (newWi == MAX_SIZE) {
                newWi = 0;
            }
        }

        m_writeIdx.store(newWi, std::memory_order_release);
        return last;
    }

    void commit_first_write_array(std::size_t n)
    {
        YOGI_ASSERT(n <= boost::asio::buffer_size(first_write_array()));

        auto wi = m_writeIdx.load(std::memory_order_relaxed);
        auto ri = m_readIdx.load(std::memory_order_acquire);

        wi += n;
        if (wi>= MAX_SIZE) {
            wi -= MAX_SIZE;
        }

        m_writeIdx.store(wi, std::memory_order_release);
    }

    boost::asio::mutable_buffers_1 first_write_array()
    {
        auto wi = m_writeIdx.load(std::memory_order_relaxed);
        auto ri = m_readIdx.load(std::memory_order_relaxed);

        if (wi < ri) {
            return boost::asio::buffer(m_data.data() + wi, ri - wi- 1);
        }

        return boost::asio::buffer(m_data.data() + wi,
            MAX_SIZE - wi - (ri == 0 ? 1 : 0));
    }
};

} // namespace base
} // namespace yogi

#endif // YOGI_BASE_LOCKFREERINGBUFFER_HPP
