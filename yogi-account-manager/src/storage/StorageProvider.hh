#ifndef STORAGE_STORAGEPROVIDER_HH
#define STORAGE_STORAGEPROVIDER_HH

#include <yogi.hpp>

#include <boost/asio/io_service.hpp>


namespace storage {

class StorageProvider
{
public:
    typedef std::vector<char> byte_array;

    static std::unique_ptr<StorageProvider> create_provider(boost::asio::io_service& ios,
        const std::string& configPath);

    StorageProvider(boost::asio::io_service& ios);
    virtual ~StorageProvider();

    boost::asio::io_service& io_service();

    virtual void async_clear(std::function<void (bool success)> completionHandler) =0;
    virtual void async_clear_section(const std::string& section,
        std::function<void (bool success)> completionHandler) =0;
    virtual void async_read_data(const std::string& section, const std::string& variable,
        std::function<void (bool success, byte_array data)> completionHandler) =0;
    virtual void async_write_data(const std::string& section, const std::string& variable,
        const byte_array& data, std::function<void (bool success)> completionHandler) =0;

protected:
    yogi::Logger& logger();

private:
    boost::asio::io_service&  m_ios;
    yogi::Logger              m_logger;
};

typedef std::unique_ptr<StorageProvider> storage_provider_ptr;

} // namespace storage

#endif // STORAGE_STORAGEPROVIDER_HH
