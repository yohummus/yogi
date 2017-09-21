#ifndef STORAGE_STORAGEINTERFACE_HH
#define STORAGE_STORAGEINTERFACE_HH

#include <yogi.hpp>

#include "StorageProvider.hh"
#include "../proto/yogi_0000d1e1.h"
#include "../proto/yogi_001ed001.h"

#include <boost/asio/io_service.hpp>


namespace storage {

class StorageInterface
{
public:
    StorageInterface(boost::asio::io_service& ios, storage::StorageProvider& storageProvider,
        yogi::Logger& logger, const yogi::Path& path, const std::string& section);

private:
    typedef yogi::ServiceTerminal<yogi_001ed001>          write_data_terminal;
    typedef yogi::MessageObserver<write_data_terminal>    write_data_observer;
    typedef std::shared_ptr<write_data_terminal::Request> write_data_request;
    typedef yogi::ServiceTerminal<yogi_0000d1e1>          read_data_terminal;
    typedef yogi::MessageObserver<read_data_terminal>     read_data_observer;
    typedef std::shared_ptr<read_data_terminal::Request>  read_data_request;

    boost::asio::io_service&  m_ios;
    storage::StorageProvider& m_storageProvider;
    yogi::Logger&             m_logger;
    const std::string         m_section;
    write_data_terminal       m_writeDataTerminal;
    write_data_observer       m_writeDataObserver;
    read_data_terminal        m_readDataTerminal;
    read_data_observer        m_readDataObserver;

    void setup_observers();
    void on_write_data_request_received(write_data_request req);
    void on_read_data_request_received(read_data_request req);
};

} // namespace storage

#endif // STORAGE_STORAGEINTERFACE_HH
