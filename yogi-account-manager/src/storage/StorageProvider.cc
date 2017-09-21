#include "StorageProvider.hh"
#include "FilesystemStorageProvider.hh"

using namespace std::string_literals;


namespace storage {

storage_provider_ptr StorageProvider::create_provider(
    boost::asio::io_service& ios, const std::string& configPath)
{
    auto type = yogi::ProcessInterface::config().get<std::string>(configPath + ".type");
    if (type == "filesystem") {
        return std::make_unique<FilesystemStorageProvider>(ios,
            yogi::ProcessInterface::config().get<std::string>(configPath + ".path"));
    }
    else {
        throw std::runtime_error("Unknown storage provider type '"s + type + "'");
    }
}

StorageProvider::StorageProvider(boost::asio::io_service& ios)
: m_ios(ios)
, m_logger("StorageProvider")
{
}

StorageProvider::~StorageProvider()
{
}

boost::asio::io_service& StorageProvider::io_service()
{
    return m_ios;
}

yogi::Logger& StorageProvider::logger()
{
    return m_logger;
}

} // namespace storage
