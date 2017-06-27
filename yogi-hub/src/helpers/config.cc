#include "config.hh"


namespace helpers {

QMap<QString, QString> extract_map_from_config(const yogi::ConfigurationChild& config, const char* childName)
{
    QMap<QString, QString> map;
    for (auto entry : config.get_child(childName)) {
        map[QString::fromStdString(entry.first)] = QString::fromStdString(entry.second.get_value<std::string>());
    }

    return map;
}

} // namespace helpers
