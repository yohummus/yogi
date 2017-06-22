#ifndef HELPERS_READ_FROM_STREAM_HH
#define HELPERS_READ_FROM_STREAM_HH

#include <yogi.hpp>

#include <QDataStream>


namespace helpers {
namespace internal {

template <typename Final, typename Intermediate>
inline Final read_from_stream_impl(QDataStream* stream)
{
    Intermediate val;
    (*stream) >> val;
    return static_cast<Final>(val);
}

} // namespace internal

template <typename T>
inline T read_from_stream(QDataStream* stream);

template <>
inline bool read_from_stream<bool>(QDataStream* stream)
{
    return internal::read_from_stream_impl<qint8, qint8>(stream) != 0;
}

template <>
inline int read_from_stream<int>(QDataStream* stream)
{
    stream->setByteOrder(QDataStream::LittleEndian);
    return internal::read_from_stream_impl<int, qint32>(stream);
}

template <>
inline unsigned read_from_stream<unsigned>(QDataStream* stream)
{
    stream->setByteOrder(QDataStream::LittleEndian);
    return internal::read_from_stream_impl<unsigned, quint32>(stream);
}

template <>
inline yogi::terminal_type read_from_stream<yogi::terminal_type>(QDataStream* stream)
{
    return internal::read_from_stream_impl<yogi::terminal_type, qint8>(stream);
}

template <>
inline yogi::Signature read_from_stream<yogi::Signature>(QDataStream* stream)
{
    return yogi::Signature(read_from_stream<unsigned>(stream));
}

} // namespace helpers

#endif // HELPERS_READ_FROM_STREAM_HH
