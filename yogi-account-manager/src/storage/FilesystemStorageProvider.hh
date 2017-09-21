#ifndef STORAGE_FILESTORAGEPROVIDER_HH
#define STORAGE_FILESTORAGEPROVIDER_HH

#include "StorageProvider.hh"

#include <boost/filesystem/path.hpp>


namespace storage {

class FilesystemStorageProvider : public StorageProvider
{
public:
    FilesystemStorageProvider(boost::asio::io_service& ios, const std::string& path);

    virtual void async_clear(std::function<void (bool success)> completionHandler) override;
    virtual void async_clear_section(const std::string& section,
        std::function<void (bool success)> completionHandler) override;
    virtual void async_read_data(const std::string& section, const std::string& variable,
        std::function<void (bool success, byte_array data)> completionHandler) override;
    virtual void async_write_data(const std::string& section, const std::string& variable,
        const byte_array& data, std::function<void (bool success)> completionHandler) override;

private:
    const boost::filesystem::path m_path;
    yogi::Error                   m_readDataError;
    yogi::Error                   m_writeDataError;

    void create_root_directory();
    void security_check_paths(const std::string& section, const std::string& variable);
};

} // namespace storage

#endif // STORAGE_FILESTORAGEPROVIDER_HH
