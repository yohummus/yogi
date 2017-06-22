#include "configuration.hpp"
#include "internal/cmdline.hpp"
#include "internal/glob.hpp"
#include "internal/ptree.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace pt = boost::property_tree;

#include <chrono>
#include <sstream>
using namespace std::string_literals;


namespace yogi {

BadCommandLine::BadCommandLine(const std::string& path)
: std::runtime_error("Bad command line: "s + path)
{
}

BadConfigurationPath::BadConfigurationPath(const std::string& path)
: std::runtime_error("Configuration child '"s + path + "' not found")
{
}

BadConfigurationDataAccess::BadConfigurationDataAccess()
: std::runtime_error("Configuration child has no data or the data cannot be converted to the requested type")
{
}

BadConfiguration::BadConfiguration(const std::string& description)
: std::runtime_error("Cannot parse configuration: " + description)
{
}

BadConfigurationFilePattern::BadConfigurationFilePattern(const std::string& pattern)
: std::runtime_error("Could not find any configuration files matching '"s + pattern + "'")
{
}

struct ConfigurationChild::Implementation {
    std::shared_ptr<pt::ptree> root;
    const pt::ptree*           child;
    bool                       valid;
};

struct ConfigurationChild::Iterator::Implementation {
    pt::ptree::const_iterator                  iterator;
    pt::ptree::const_iterator                  end;
    std::pair<std::string, ConfigurationChild> value;
};

ConfigurationChild::Iterator::Iterator()
: m_impl(std::make_unique<Implementation>())
{
}

ConfigurationChild::Iterator::Iterator(const Iterator& other)
: m_impl(std::make_unique<Implementation>())
{
    *this = other;
}

ConfigurationChild::Iterator::Iterator(Iterator&& other)
{
    *this = std::move(other);
    assert (m_impl->iterator != pt::ptree::const_iterator());
}

ConfigurationChild::Iterator::~Iterator()
{
}

ConfigurationChild::Iterator& ConfigurationChild::Iterator::operator= (const Iterator& other)
{
    *m_impl = *other.m_impl;
    return *this;
}

ConfigurationChild::Iterator& ConfigurationChild::Iterator::operator= (Iterator&& other)
{
    m_impl = std::move(other.m_impl);
    return *this;
}

bool ConfigurationChild::Iterator::operator== (const Iterator& other) const
{
    return m_impl->iterator == other.m_impl->iterator;
}

bool ConfigurationChild::Iterator::operator!= (const Iterator& other) const
{
    return !(*this == other);
}

ConfigurationChild::Iterator& ConfigurationChild::Iterator::operator++ ()
{
    ++m_impl->iterator;
    if (m_impl->iterator == m_impl->end) {
        m_impl->value = decltype(m_impl->value)();
    }
    else {
        ConfigurationChild child(m_impl->value.second);
        child.m_impl->child = &m_impl->iterator->second;
        m_impl->value = std::make_pair(m_impl->iterator->first, child);
    }

    return *this;
}

ConfigurationChild::Iterator ConfigurationChild::Iterator::operator++ (int)
{
    Iterator it(*this);
    ++(*this);
    return it;
}

const std::pair<std::string, ConfigurationChild>& ConfigurationChild::Iterator::operator* () const
{
    assert (m_impl->iterator != pt::ptree::const_iterator());
    return m_impl->value;
}

const std::pair<std::string, ConfigurationChild>* ConfigurationChild::Iterator::operator-> () const
{
    assert (m_impl->iterator != pt::ptree::const_iterator());
    return &m_impl->value;
}

ConfigurationChild::ConfigurationChild()
: m_impl(std::make_unique<Implementation>())
{
    m_impl->root = std::make_shared<pt::ptree>();
    m_impl->child = &*m_impl->root;
    m_impl->valid = false;
}

ConfigurationChild::ConfigurationChild(const ConfigurationChild& other)
: ConfigurationChild()
{
    *this = other;
}

ConfigurationChild::ConfigurationChild(ConfigurationChild&& other)
{
    *this = std::move(other);
}

ConfigurationChild::~ConfigurationChild()
{
}

ConfigurationChild& ConfigurationChild::operator= (const ConfigurationChild& other)
{
    *m_impl = *other.m_impl;
    return *this;
}

ConfigurationChild& ConfigurationChild::operator= (ConfigurationChild&& other)
{
    m_impl = std::move(other.m_impl);
    return *this;
}

#define IMPLEMENT_GET_VALUE_OPTIONAL(type)                              \
    template<>                                                          \
    Optional<type> ConfigurationChild::get_value_optional<type>() const \
    {                                                                   \
        if (!m_impl->valid) {                                           \
            return Optional<type>();                                    \
        }                                                               \
                                                                        \
        auto val = m_impl->child->get_value_optional<type>();           \
        return val ? Optional<type>(*val) : Optional<type>();           \
    }

IMPLEMENT_GET_VALUE_OPTIONAL(bool)
IMPLEMENT_GET_VALUE_OPTIONAL(char)
IMPLEMENT_GET_VALUE_OPTIONAL(unsigned char)
IMPLEMENT_GET_VALUE_OPTIONAL(short)
IMPLEMENT_GET_VALUE_OPTIONAL(unsigned short)
IMPLEMENT_GET_VALUE_OPTIONAL(int)
IMPLEMENT_GET_VALUE_OPTIONAL(unsigned int)
IMPLEMENT_GET_VALUE_OPTIONAL(long)
IMPLEMENT_GET_VALUE_OPTIONAL(unsigned long)
IMPLEMENT_GET_VALUE_OPTIONAL(long long)
IMPLEMENT_GET_VALUE_OPTIONAL(unsigned long long)
IMPLEMENT_GET_VALUE_OPTIONAL(float)
IMPLEMENT_GET_VALUE_OPTIONAL(double)
IMPLEMENT_GET_VALUE_OPTIONAL(std::string)

template<>
Optional<std::chrono::milliseconds> ConfigurationChild::get_value_optional<std::chrono::milliseconds>() const
{
    typedef std::chrono::milliseconds type;

    auto val = get_value_optional<float>();
    if (val) {
        auto dur = *val < 0 ? type::max() : type(static_cast<type::rep>(*val * 1000));
        return Optional<type>(dur);
    }
    else {
        return Optional<type>();
    }
}

ConfigurationChild ConfigurationChild::get_child(const std::string& path) const
{
    auto child = get_child_optional(path);
    if (!child) {
        throw BadConfigurationPath(path);
    }

    return std::move(*child);
}

ConfigurationChild ConfigurationChild::get_child(const std::string& path, const ConfigurationChild& defaultChild) const
{
    auto child = get_child_optional(path);
    if (!child) {
        return defaultChild;
    }

    return *child;
}

Optional<ConfigurationChild> ConfigurationChild::get_child_optional(const std::string& path) const
{
    if (!m_impl->valid) {
        return Optional<ConfigurationChild>();
    }

    auto ptree = m_impl->child->get_child_optional(path);
    if (!ptree) {
        return Optional<ConfigurationChild>();
    }

    ConfigurationChild child(*this);
    child.m_impl->child = &*ptree;
    return child;
}

std::size_t ConfigurationChild::size() const
{
    return m_impl->child->size();
}

bool ConfigurationChild::empty() const
{
    return m_impl->child->empty();
}

ConfigurationChild::Iterator ConfigurationChild::begin() const
{
    auto ptreeIt = m_impl->child->begin();

    if (ptreeIt == m_impl->child->end()) {
        return end();
    }

    ConfigurationChild child;
    child.m_impl->root  = m_impl->root;
    child.m_impl->child = &ptreeIt->second;
    child.m_impl->valid = true;

    Iterator it;
    it.m_impl->iterator = ptreeIt;
    it.m_impl->end      = m_impl->child->end();
    it.m_impl->value    = std::make_pair(ptreeIt->first, child);

    return it;
}

ConfigurationChild::Iterator ConfigurationChild::end() const
{
    Iterator it;
    it.m_impl->iterator = m_impl->child->end();
    it.m_impl->end      = m_impl->child->end();

    return it;
}

std::string ConfigurationChild::to_string() const
{
    std::stringstream ss;
    pt::write_json(ss, *m_impl->root);
    // TODO: using m_impl->child does not work because boost throws the following runtime error:
    //       ERROR: <unspecified file>: ptree contains data that cannot be represented in JSON format
    return ss.str();
}

struct Configuration::Implementation {
    Path location;
};

Configuration::Configuration()
: m_impl(std::make_unique<Implementation>())
{
    *ConfigurationChild::m_impl->root = pt::ptree();
    ConfigurationChild::m_impl->valid = true;
}

Configuration::Configuration(int argc, const char* const argv[], bool configFileRequired)
: Configuration()
{
    internal::CommandLineOptions opts;

    try {
        opts = internal::parse_cmdline(argc, argv, configFileRequired);
    }
    catch(const std::exception& e) {
        throw BadCommandLine(e.what());
    }

    auto configFiles = internal::glob(opts.configFilePatterns);

    try {
        *ConfigurationChild::m_impl->root = internal::parse_json_files(configFiles);
        ConfigurationChild::m_impl->valid = true;
    }
    catch (const std::exception& e) {
        throw BadConfiguration(e.what());
    }

    internal::update_ptree(&*ConfigurationChild::m_impl->root, opts);
    m_impl->location = this->get<std::string>("yogi.location", "/");
}

Configuration::Configuration(const Configuration& other)
: Configuration()
{
    *this = other;
}

Configuration::Configuration(Configuration&& other)
{
    *this = std::move(other);
}

Configuration::~Configuration()
{
}

Configuration& Configuration::operator= (const Configuration& other)
{
    *m_impl = *other.m_impl;
    ConfigurationChild::operator= (other);
    return *this;
}

Configuration& Configuration::operator= (Configuration&& other)
{
    m_impl = std::move(other.m_impl);
    ConfigurationChild::operator= (std::move(other));
    return *this;
}

const Path& Configuration::location() const
{
    return m_impl->location;
}

Optional<std::string> Configuration::connection_target() const
{
    return this->get_optional<std::string>("yogi.connection.target");
}

std::chrono::milliseconds Configuration::connection_timeout() const
{
    auto timeout = this->get("yogi.connection.timeout", -1.0);
    if (timeout < 0) {
        return std::chrono::milliseconds::max();
    }
    else {
        return std::chrono::milliseconds(static_cast<int>(timeout * 1000));
    }
}

Optional<std::string> Configuration::connection_identification() const
{
    return this->get_optional<std::string>("yogi.connection.identification");
}

void Configuration::update(const std::string& json)
{
    pt::ptree partialTree;

    try {
        std::stringstream ss(json);
        pt::read_json(ss, partialTree);
    }
    catch (const std::exception& e) {
        throw BadConfiguration(e.what());
    }

    internal::merge_property_trees(&*ConfigurationChild::m_impl->root, partialTree);
    m_impl->location = this->get<std::string>("yogi.location", "/");
}

} // namespace yogi

std::ostream& operator<< (std::ostream& os, const yogi::ConfigurationChild& child)
{
    return os << child.to_string();
}
