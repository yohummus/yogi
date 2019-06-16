/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2019 Johannes Bergmann.
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

#include "../common.h"
#include "../../src/utils/ringbuffer.h"
using namespace utils;

class RingBufferTest : public TestFixture {
 protected:
  LockFreeRingBuffer uut{10};

  Byte GetFirstReadArrayElement(std::size_t idx) const {
    ByteVector data(idx + 1);
    boost::asio::buffer_copy(boost::asio::buffer(data), uut.FirstReadArray());
    return data.back();
  }

  void SetFirstWriteArrayElement(std::size_t idx, Byte byte) {
    boost::asio::buffer_copy(uut.FirstWriteArray() + idx,
                             boost::asio::buffer(&byte, 1));
  }

  std::size_t FirstReadArraySize() const {
    return boost::asio::buffer_size(uut.FirstReadArray());
  }

  std::size_t FirstWriteArraySize() {
    return boost::asio::buffer_size(uut.FirstWriteArray());
  }
};

TEST_F(RingBufferTest, FirstReadArray) {
  ByteVector data(uut.Capacity(), 0);
  data[0] = 1;
  data[1] = 2;
  data[2] = 3;
  data.back() = 4;

  auto n = uut.Write(data.data(), data.size());
  EXPECT_EQ(n, data.size());
  EXPECT_EQ(uut.Capacity(), FirstReadArraySize());
  EXPECT_EQ(1, GetFirstReadArrayElement(0));
  EXPECT_EQ(4, GetFirstReadArrayElement(uut.Capacity() - 1));

  uut.CommitFirstReadArray(2);
  EXPECT_EQ(uut.Capacity() - 2, FirstReadArraySize());
  EXPECT_EQ(3, GetFirstReadArrayElement(0));
  EXPECT_EQ(4, GetFirstReadArrayElement(uut.Capacity() - 3));

  uut.Write(data.data(), 2);
  EXPECT_EQ(uut.Capacity() - 1, FirstReadArraySize());
  EXPECT_EQ(3, GetFirstReadArrayElement(0));
  EXPECT_EQ(4, GetFirstReadArrayElement(uut.Capacity() - 3));
  EXPECT_EQ(1, GetFirstReadArrayElement(uut.Capacity() - 2));

  uut.CommitFirstReadArray(uut.Capacity() - 1);
  EXPECT_EQ(1, FirstReadArraySize());
  EXPECT_EQ(2, GetFirstReadArrayElement(0));
}

TEST_F(RingBufferTest, FirstWriteArray) {
  EXPECT_EQ(uut.Capacity(), FirstWriteArraySize());
  SetFirstWriteArrayElement(0, 1);
  SetFirstWriteArrayElement(1, 2);
  uut.CommitFirstWriteArray(2);

  Byte data[2];
  EXPECT_EQ(2, uut.Read(data, 2));
  EXPECT_EQ(1, data[0]);
  EXPECT_EQ(2, data[1]);

  EXPECT_EQ(uut.Capacity() - 1, FirstWriteArraySize());
  uut.CommitFirstWriteArray(FirstWriteArraySize());
  EXPECT_EQ(1, FirstWriteArraySize());
  uut.CommitFirstWriteArray(1);
  EXPECT_EQ(0, FirstWriteArraySize());
}

TEST_F(RingBufferTest, Empty) {
  EXPECT_TRUE(uut.Empty());
  ByteVector buffer{'x'};
  uut.Write(buffer.data(), buffer.size());
  EXPECT_FALSE(uut.Empty());
  uut.Discard(1);
  EXPECT_TRUE(uut.Empty());
}

TEST_F(RingBufferTest, Full) {
  EXPECT_FALSE(uut.Full());
  ByteVector buffer(uut.Capacity() - 1, 'x');
  uut.Write(buffer.data(), buffer.size());
  EXPECT_FALSE(uut.Full());
  uut.Write(buffer.data(), 1);
  uut.Discard(1);
  EXPECT_FALSE(uut.Full());
}

TEST_F(RingBufferTest, FrontAndPop) {
  ByteVector buffer{'a', 'b', 'c'};
  uut.Write(buffer.data(), buffer.size());
  EXPECT_EQ('a', uut.Front());
  uut.Pop();
  EXPECT_EQ('b', uut.Front());
  uut.Pop();
  EXPECT_EQ('c', uut.Front());
  uut.Pop();
  EXPECT_TRUE(uut.Empty());
}

TEST_F(RingBufferTest, PopUntil) {
  ByteVector buffer{'a', 'b', 'c', 'd', 'e'};
  uut.Write(buffer.data(), buffer.size());

  uut.PopUntil([&](auto byte) {
    return byte == 'b';
  });
  EXPECT_EQ('c', uut.Front());

  uut.PopUntil([&](auto) {
    return false;
  });
  EXPECT_TRUE(uut.Empty());
}

TEST_F(RingBufferTest, AvailableForRead) {
  EXPECT_EQ(uut.AvailableForRead(), 0);

  Byte byte = 'x';
  uut.Write(&byte, 1);
  uut.Write(&byte, 1);
  EXPECT_EQ(uut.AvailableForRead(), 2);
  ByteVector buffer(10);
  uut.Read(buffer.data(), buffer.size());
  EXPECT_EQ(uut.AvailableForRead(), 0);

  for (std::size_t i = 0; i < uut.Capacity(); ++i) {
    uut.Write(&byte, 1);
    EXPECT_EQ(uut.AvailableForRead(), i + 1);
  }
}

TEST_F(RingBufferTest, AvailableForWrite) {
  EXPECT_EQ(uut.AvailableForWrite(), uut.Capacity());

  Byte byte = 'x';
  uut.Write(&byte, 1);
  uut.Write(&byte, 1);
  EXPECT_EQ(uut.AvailableForWrite(), uut.Capacity() - 2);
  ByteVector buffer(10);
  uut.Read(buffer.data(), buffer.size());
  EXPECT_EQ(uut.AvailableForWrite(), uut.Capacity());

  for (std::size_t i = 0; i < uut.Capacity(); ++i) {
    uut.Write(&byte, 1);
    EXPECT_EQ(uut.AvailableForWrite(), uut.Capacity() - i - 1);
  }
}
