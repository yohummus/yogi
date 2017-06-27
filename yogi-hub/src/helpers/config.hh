#ifndef HELPERS_CONFIG_HH
#define HELPERS_CONFIG_HH

#include <yogi.hpp>

#include <QMap>
#include <QString>


namespace helpers {

QMap<QString, QString> extract_map_from_config(const yogi::ConfigurationChild& config, const char* childName);

} // namespace helpers

#endif // HELPERS_CONFIG_HH
