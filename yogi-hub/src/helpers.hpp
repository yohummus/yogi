#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <yogi.hpp>

#include <QByteArray>
#include <QDataStream>
#include <QString>

#include <chrono>
#include <string>


namespace {
namespace helpers {

std::chrono::milliseconds float_to_timeout(float val)
{
    if (val <= 0.0) {
        return std::chrono::milliseconds::max();
    }
    else {
        return std::chrono::milliseconds(static_cast<int>(val * 1000));
    }
}

template <typename T>
QByteArray to_byte_array(const T& val);

template <>
QByteArray to_byte_array<bool>(const bool& val)
{
    return QByteArray(1, val ? 1 : 0);
}

template <>
QByteArray to_byte_array<char>(const char& val)
{
    return QByteArray(1, val);
}

template <>
QByteArray to_byte_array<int>(const int& val)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream << static_cast<qint32>(val);
	return buffer;
}

template <>
QByteArray to_byte_array<unsigned>(const unsigned& val)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint32>(val);
    return buffer;
}

template <>
QByteArray to_byte_array<std::chrono::system_clock::time_point>(const std::chrono::system_clock::time_point& time)
{
    return to_byte_array(static_cast<unsigned>(std::chrono::duration_cast<std::chrono::seconds>(
        time.time_since_epoch()).count()));
}

template <>
QByteArray to_byte_array<yogi::binding_state>(const yogi::binding_state& state)
{
    return to_byte_array(state == yogi::binding_state::ESTABLISHED);
}

template <>
QByteArray to_byte_array<yogi::subscription_state>(const yogi::subscription_state& state)
{
    return to_byte_array(state == yogi::subscription_state::SUBSCRIBED);
}

template <>
QByteArray to_byte_array<yogi::gather_flags>(const yogi::gather_flags& flags)
{
    return to_byte_array(static_cast<char>(flags));
}

template <>
QByteArray to_byte_array<yogi::change_type>(const yogi::change_type& change)
{
    return to_byte_array(change == yogi::change_type::ADDED);
}

template <>
QByteArray to_byte_array<yogi::terminal_type>(const yogi::terminal_type& type)
{
    return QByteArray(1, static_cast<char>(type));
}

template <>
QByteArray to_byte_array<yogi::Signature>(const yogi::Signature& signature)
{
    return to_byte_array(signature.raw());
}

template <>
QByteArray to_byte_array<std::string>(const std::string& str)
{
    return QByteArray(str.c_str(), static_cast<int>(str.size() + 1));
}

template <>
QByteArray to_byte_array<QString>(const QString& str)
{
    return to_byte_array(str.toStdString());
}

template <typename Final, typename Intermediate>
inline Final read_from_stream_impl(QDataStream* stream)
{
    Intermediate val;
    (*stream) >> val;
    return static_cast<Final>(val);
}

template <typename T>
inline T read_from_stream(QDataStream* stream);

template <>
inline bool read_from_stream<bool>(QDataStream* stream)
{
    return read_from_stream_impl<qint8, qint8>(stream) != 0;
}

template <>
inline int read_from_stream<int>(QDataStream* stream)
{
    stream->setByteOrder(QDataStream::LittleEndian);
    return read_from_stream_impl<int, qint32>(stream);
}

template <>
inline unsigned read_from_stream<unsigned>(QDataStream* stream)
{
    stream->setByteOrder(QDataStream::LittleEndian);
    return read_from_stream_impl<unsigned, quint32>(stream);
}

template <>
inline yogi::terminal_type read_from_stream<yogi::terminal_type>(QDataStream* stream)
{
    return read_from_stream_impl<yogi::terminal_type, qint8>(stream);
}

template <>
inline yogi::Signature read_from_stream<yogi::Signature>(QDataStream* stream)
{
    return yogi::Signature(read_from_stream<unsigned>(stream));
}

} // namespace helpers
} // anonymous namespace

inline std::ostream& operator<< (std::ostream& os, const QString& str)
{
    return os << str.toStdString();
}

#endif // HELPERS_HPP
