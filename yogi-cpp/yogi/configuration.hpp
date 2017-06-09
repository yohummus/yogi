#ifndef YOGI_CONFIGURATION_HPP
#define YOGI_CONFIGURATION_HPP

#include "optional.hpp"
#include "path.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <exception>


namespace yogi {

class BadCommandLine : public std::runtime_error
{
public:
    BadCommandLine(const std::string& description);
};


class BadConfigurationPath : public std::runtime_error
{
public:
    BadConfigurationPath(const std::string& path);
};


class BadConfigurationDataAccess : public std::runtime_error
{
public:
    BadConfigurationDataAccess();
};


class BadConfiguration : public std::runtime_error
{
public:
    BadConfiguration(const std::string& description);
};


class BadConfigurationFilePattern : public std::runtime_error
{
public:
    BadConfigurationFilePattern(const std::string& pattern);
};


class Configuration;


class ConfigurationChild
{
    friend class Configuration;

    struct Implementation;

public:
    class Iterator
    {
        friend class ConfigurationChild;

        struct Implementation;

    private:
        std::unique_ptr<Implementation> m_impl;

    private:
        Iterator();

    public:
        Iterator(const Iterator& other);
        Iterator(Iterator&& other);
        ~Iterator();
        Iterator& operator= (const Iterator& other);
        Iterator& operator= (Iterator&& other);

        bool operator== (const Iterator& other) const;
        bool operator!= (const Iterator& other) const;

        Iterator& operator++ ();
        Iterator operator++ (int);

        const std::pair<std::string, ConfigurationChild>& operator* () const;
        const std::pair<std::string, ConfigurationChild>* operator-> () const;
    };

protected:
    std::unique_ptr<Implementation> m_impl;

public:
    ConfigurationChild();
    ConfigurationChild(const ConfigurationChild& other);
    ConfigurationChild(ConfigurationChild&& other);
    ~ConfigurationChild();

    ConfigurationChild& operator= (const ConfigurationChild& other);
    ConfigurationChild& operator= (ConfigurationChild&& other);

    template <typename T> T get_value() const;
    template <typename T> T get_value(const T& defaultValue) const;
    template <typename T> Optional<T> get_value_optional() const;

    ConfigurationChild get_child(const std::string& path) const;
    ConfigurationChild get_child(const std::string& path, const ConfigurationChild& defaultChild) const;
    Optional<ConfigurationChild> get_child_optional(const std::string& path) const;

    template <typename T> T get(const std::string& path) const;
    template <typename T> T get(const std::string& path, const T& defaultValue) const;
    template <typename T> Optional<T> get_optional(const std::string& path) const;

    std::size_t size() const;
    bool empty() const;
    Iterator begin() const;
    Iterator end() const;

    std::string to_string() const;
};


class Configuration final : public ConfigurationChild
{
    struct Implementation;

private:
    std::unique_ptr<Implementation> m_impl;

public:
    Configuration();
    Configuration(int argc, const char* const argv[], bool configFileRequired = false);
    Configuration(const Configuration& other);
    Configuration(Configuration&& other);
    ~Configuration();

    Configuration& operator= (const Configuration& other);
    Configuration& operator= (Configuration&& other);

    const Path&               location() const;
    Optional<std::string>     connection_target() const;
    std::chrono::milliseconds connection_timeout() const;
    Optional<std::string>     connection_identification() const;

    void update(const std::string& json);
};


template <typename T>
T ConfigurationChild::get_value() const
{
    if (auto val = get_value_optional<T>()) {
        return *val;
    }

    throw BadConfigurationDataAccess();
}

template <typename T>
T ConfigurationChild::get_value(const T& defaultValue) const
{
    if (auto val = get_value_optional<T>()) {
        return *val;
    }

    return defaultValue;
}

template <typename T>
T ConfigurationChild::get(const std::string& path) const
{
    return get_child(path).get_value<T>();
}

template <typename T>
T ConfigurationChild::get(const std::string& path, const T& defaultValue) const
{
    return get_child(path, ConfigurationChild()).get_value<T>(defaultValue);
}

template <typename T>
Optional<T> ConfigurationChild::get_optional(const std::string& path) const
{
    if (auto child = get_child_optional(path)) {
        return child->get_value_optional<T>();
    }

    return Optional<T>();
}

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::ConfigurationChild& child);

#endif // YOGI_CONFIGURATION_HPP
