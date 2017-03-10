#include "../../src/serialization/serialize.hpp"
#include "../../src/serialization/deserialize.hpp"
#include "../../src/serialization/can_deserialize_one.hpp"
using namespace chirp;

#include <gmock/gmock.h>


struct SerializationTest : public testing::Test
{
    std::vector<char> buffer;
};

TEST_F(SerializationTest, SerializeDeserialize)
{
    serialization::serialize(buffer, std::size_t{123}, std::size_t{456});
    buffer.push_back(123);
    std::size_t a;
    std::size_t b;
    auto it = serialization::deserialize(buffer, buffer.begin(), a, b);
    EXPECT_EQ(buffer.end() - 1, it);
    EXPECT_EQ(123, a);
    EXPECT_EQ(456, b);
}

TEST_F(SerializationTest, Size)
{
    struct value_t {
        std::size_t serializedLength;
        std::size_t value;
    };

    // those are all the corner cases
    const value_t entries[] = {
        {1, 0ul},
        {1, 127ul},
        {2, 128ul},
        {2, 16383ul},
        {3, 16384ul},
        {3, 2097151ul},
        {4, 2097152ul},
        {4, 268435455ul},
        {5, 268435456ul},
        {5, 4294967295ul}
    };

    for (auto entry : entries) {
        buffer.resize(1, 'x');

        serialization::serialize_one(buffer, entry.value);
        EXPECT_EQ(1 + entry.serializedLength, buffer.size());
        EXPECT_EQ('x', buffer[0]);

        buffer.resize(buffer.size() + 1);
        buffer.back() = 'y';

        std::size_t value;
        auto it = buffer.cbegin() + 1;
        serialization::deserialize_one(buffer, it, value);
        EXPECT_EQ(it, buffer.end() - 1);
        EXPECT_EQ(entry.value, value);
    }

    buffer.resize(0);
    serialization::serialize_one(buffer, std::size_t{16384ul});
    EXPECT_TRUE(serialization::can_deserialize_one<std::size_t>(
        buffer, buffer.begin()));
    buffer.resize(buffer.size() - 1);
    EXPECT_FALSE(serialization::can_deserialize_one<std::size_t>(
        buffer, buffer.begin()));
}

TEST_F(SerializationTest, Id)
{
    buffer.resize(1, 'x');

    serialization::serialize_one(buffer, base::Id{12345});
    EXPECT_GT(buffer.size(), 1ul);
    EXPECT_EQ('x', buffer[0]);

    buffer.resize(buffer.size() + 1);
    buffer.back() = 'y';

    base::Id id;
    auto it = buffer.cbegin() + 1;
    serialization::deserialize_one(buffer, it, id);
    EXPECT_EQ(it, buffer.end() - 1);
    EXPECT_EQ(base::Id{12345}, id);
}

TEST_F(SerializationTest, Identifier)
{
    buffer.resize(1, 'x');

    serialization::serialize_one(buffer, base::Identifier{12345ul, "Hello", true});
    EXPECT_GT(buffer.size(), 1ul);
    EXPECT_EQ('x', buffer[0]);
    EXPECT_NE('\0', buffer.back());

    buffer.resize(buffer.size() + 1);
    buffer.back() = 'y';

    base::Identifier identifier;
    auto it = buffer.cbegin() + 1;
    serialization::deserialize_one(buffer, it, identifier);
    EXPECT_EQ(it, buffer.end() - 1);
    EXPECT_EQ(12345ul, identifier.signature());
    EXPECT_EQ("Hello", identifier.name());
    EXPECT_TRUE(identifier.hidden());
}

TEST_F(SerializationTest, Buffer)
{
    buffer.resize(1, 'x');

    base::Buffer origBuf{"test", 4};
    serialization::serialize_one(buffer, origBuf);
    EXPECT_GT(buffer.size(), 1ul);
    EXPECT_EQ('x', buffer[0]);

    buffer.resize(buffer.size() + 1);
    buffer.back() = 'y';

    base::Buffer buf;
    auto it = buffer.cbegin() + 1;
    serialization::deserialize_one(buffer, it, buf);
    EXPECT_EQ(it, buffer.end() - 1);
    EXPECT_EQ(origBuf, buf);
}

TEST_F(SerializationTest, GatherFlags)
{
    buffer.resize(1, 'x');

    core::scatter_gather::gather_flags origFlags
        = core::scatter_gather::GATHER_DEAF
        | core::scatter_gather::GATHER_IGNORED;
    serialization::serialize_one(buffer, origFlags);
    EXPECT_EQ(2, buffer.size());
    EXPECT_EQ('x', buffer[0]);

    buffer.resize(buffer.size() + 1);
    buffer.back() = 'y';

    core::scatter_gather::gather_flags flags;
    auto it = buffer.cbegin() + 1;
    serialization::deserialize_one(buffer, it, flags);
    EXPECT_EQ(it, buffer.end() - 1);
    EXPECT_EQ(origFlags, flags);
}
