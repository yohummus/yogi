#ifndef HELPERS_TO_BYTE_ARRAY_HH
#define HELPERS_TO_BYTE_ARRAY_HH

#include <yogi.hpp>

#include <QByteArray>
#include <QDataStream>
#include <QString>

#include <chrono>
#include <string>


namespace helpers {

template <typename T>
inline QByteArray to_byte_array(const T& val);

template <>
inline QByteArray to_byte_array<bool>(const bool& val)
{
    return QByteArray(1, val ? 1 : 0);
}

template <>
inline QByteArray to_byte_array<char>(const char& val)
{
    return QByteArray(1, val);
}

template <>
inline QByteArray to_byte_array<int>(const int& val)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream << static_cast<qint32>(val);
	return buffer;
}

template <>
inline QByteArray to_byte_array<unsigned>(const unsigned& val)
{
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint32>(val);
    return buffer;
}

template <>
inline QByteArray to_byte_array<std::chrono::system_clock::time_point>(const std::chrono::system_clock::time_point& time)
{
    return to_byte_array(static_cast<unsigned>(std::chrono::duration_cast<std::chrono::seconds>(
        time.time_since_epoch()).count()));
}

template <>
inline QByteArray to_byte_array<yogi::binding_state>(const yogi::binding_state& state)
{
    return to_byte_array(state == yogi::binding_state::ESTABLISHED);
}

template <>
inline QByteArray to_byte_array<yogi::subscription_state>(const yogi::subscription_state& state)
{
    return to_byte_array(state == yogi::subscription_state::SUBSCRIBED);
}

template <>
inline QByteArray to_byte_array<yogi::gather_flags>(const yogi::gather_flags& flags)
{
    return to_byte_array(static_cast<char>(flags));
}

template <>
inline QByteArray to_byte_array<yogi::change_type>(const yogi::change_type& change)
{
    return to_byte_array(change == yogi::change_type::ADDED);
}

template <>
inline QByteArray to_byte_array<yogi::terminal_type>(const yogi::terminal_type& type)
{
    return QByteArray(1, static_cast<char>(type));
}

template <>
inline QByteArray to_byte_array<yogi::Signature>(const yogi::Signature& signature)
{
    return to_byte_array(signature.raw());
}

template <>
inline QByteArray to_byte_array<std::string>(const std::string& str)
{
    return QByteArray(str.c_str(), static_cast<int>(str.size() + 1));
}

template <>
inline QByteArray to_byte_array<QString>(const QString& str)
{
    return to_byte_array(str.toStdString());
}

} // namespace helpers

#endif // HELPERS_TO_BYTE_ARRAY_HH
