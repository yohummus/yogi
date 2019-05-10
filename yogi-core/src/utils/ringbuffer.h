/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../config.h"
#include "types.h"

#include <boost/asio/buffer.hpp>
#include <atomic>
#include <vector>
#include <memory>

namespace utils {

// Implementation based on the lock-free single-producer/single-consumer
// ringbuffer implementation in boost by Tim Blechmann (spsc_queue).
class LockFreeRingBuffer {
 public:
  explicit LockFreeRingBuffer(std::size_t capacity);

  std::size_t Capacity() const { return capacity_; };
  bool Empty();
  bool Full();
  Byte Front() const;
  void Pop();
  std::size_t AvailableForRead() const;
  std::size_t Read(Byte* buffer, std::size_t max_size);
  std::size_t Discard(std::size_t max_size);
  void CommitFirstReadArray(std::size_t n);
  boost::asio::const_buffers_1 FirstReadArray() const;
  std::size_t AvailableForWrite() const;
  std::size_t Write(const Byte* data, std::size_t size);
  void CommitFirstWriteArray(std::size_t n);
  boost::asio::mutable_buffers_1 FirstWriteArray();

  template <typename Fn>
  void PopUntil(Fn fn) {
    auto wi = write_idx_.load(std::memory_order_acquire);
    auto ri = read_idx_.load(std::memory_order_relaxed);

    while (AvailableForRead(wi, ri) > 0) {
      auto byte = data_[ri];
      ri = NextIndex(ri);
      if (fn(byte)) break;
    }

    read_idx_.store(ri, std::memory_order_release);
  }

 private:
  std::size_t AvailableForRead(std::size_t write_idx,
                               std::size_t read_idx) const;
  std::size_t AvailableForWrite(std::size_t write_idx,
                                std::size_t read_idx) const;
  std::size_t NextIndex(std::size_t idx) const;

  static constexpr int kCacheLineSize = 64;
  std::atomic<std::size_t> write_idx_;
  Byte padding_[kCacheLineSize - sizeof(std::size_t)];
  std::atomic<std::size_t> read_idx_;
  const std::size_t capacity_;
  ByteVector data_;
};

}  // namespace utils
