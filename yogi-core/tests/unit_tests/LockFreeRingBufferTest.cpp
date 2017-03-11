#include "../../src/base/LockFreeRingBuffer.hpp"
using namespace yogi::base;

#include <gmock/gmock.h>


struct LockFreeRingBufferTest: public testing::Test
{
    LockFreeRingBuffer uut;

    char get_first_read_array_element(std::size_t idx) const
    {
        std::vector<char> data(idx + 1);
        boost::asio::buffer_copy(boost::asio::buffer(data),
            uut.first_read_array());
        return data.back();
    }

    void set_first_write_array_element(std::size_t idx, char byte)
    {
        boost::asio::buffer_copy(uut.first_write_array() + idx,
            boost::asio::buffer(&byte, 1));
    }

    std::size_t first_read_array_size() const
    {
        return boost::asio::buffer_size(uut.first_read_array());
    }

    std::size_t first_write_array_size()
    {
        return boost::asio::buffer_size(uut.first_write_array());
    }
};

TEST_F(LockFreeRingBufferTest, FirstReadArray)
{
    std::vector<char> data(uut.capacity(), 0);
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;
    data.back() = 4;

    auto it = uut.write(data.begin(), data.end());
    EXPECT_EQ(data.end(), it);
    EXPECT_EQ(uut.capacity(), first_read_array_size());
    EXPECT_EQ(1, get_first_read_array_element(0));
    EXPECT_EQ(4, get_first_read_array_element(uut.capacity() - 1));

    uut.commit_first_read_array(2);
    EXPECT_EQ(uut.capacity() - 2, first_read_array_size());
    EXPECT_EQ(3, get_first_read_array_element(0));
    EXPECT_EQ(4, get_first_read_array_element(uut.capacity() - 3));

    uut.write(data.begin(), data.begin() + 2);
    EXPECT_EQ(uut.capacity() - 1, first_read_array_size());
    EXPECT_EQ(3, get_first_read_array_element(0));
    EXPECT_EQ(4, get_first_read_array_element(uut.capacity() - 3));
    EXPECT_EQ(1, get_first_read_array_element(uut.capacity() - 2));

    uut.commit_first_read_array(uut.capacity() - 1);
    EXPECT_EQ(1, first_read_array_size());
    EXPECT_EQ(2, get_first_read_array_element(0));
}

TEST_F(LockFreeRingBufferTest, FirstWriteArray)
{
    EXPECT_EQ(uut.capacity(), first_write_array_size());
    set_first_write_array_element(0, 1);
    set_first_write_array_element(1, 2);
    uut.commit_first_write_array(2);

	char data[2];
    EXPECT_EQ(2, uut.read(data, 2));
    EXPECT_EQ(1, data[0]);
    EXPECT_EQ(2, data[1]);

    EXPECT_EQ(uut.capacity() - 1, first_write_array_size());
    uut.commit_first_write_array(first_write_array_size());
    EXPECT_EQ(1, first_write_array_size());
    uut.commit_first_write_array(1);
    EXPECT_EQ(0, first_write_array_size());
}

TEST_F(LockFreeRingBufferTest, Empty)
{
	EXPECT_TRUE(uut.empty());
	std::vector<char> buffer{'x'};
	uut.write(buffer.begin(), buffer.end());
	EXPECT_FALSE(uut.empty());
	char byte;
	uut.read(&byte, 1);
	EXPECT_TRUE(uut.empty());
}

TEST_F(LockFreeRingBufferTest, Full)
{
	EXPECT_FALSE(uut.full());
	std::vector<char> buffer(uut.capacity() - 1, 'x');
	uut.write(buffer.begin(), buffer.end());
	EXPECT_FALSE(uut.full());
	uut.write(buffer.begin(), buffer.begin() + 1);
	char byte;
	uut.read(&byte, 1);
	EXPECT_FALSE(uut.full());
}

TEST_F(LockFreeRingBufferTest, FrontAndPop)
{
	std::vector<char> buffer{'a', 'b', 'c'};
	uut.write(buffer.begin(), buffer.end());
	EXPECT_EQ('a', uut.front());
	uut.pop();
	EXPECT_EQ('b', uut.front());
	uut.pop();
	EXPECT_EQ('c', uut.front());
	uut.pop();
	EXPECT_TRUE(uut.empty());
}
