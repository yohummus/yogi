#include "StorageInterface.hh"


namespace storage {

StorageInterface::StorageInterface(boost::asio::io_service& ios,
    storage::StorageProvider& storageProvider, yogi::Logger& logger, const yogi::Path& path,
    const std::string& section)
: m_ios(ios)
, m_storageProvider(storageProvider)
, m_logger(logger)
, m_section(section)
, m_writeDataTerminal(path / "Store Data")
, m_writeDataObserver(m_writeDataTerminal)
, m_readDataTerminal(path / "Read Data")
, m_readDataObserver(m_readDataTerminal)
{
    setup_observers();
}

void StorageInterface::setup_observers()
{
    m_writeDataObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<write_data_terminal::Request>(std::move(req));
        this->m_ios.post([=]() {
            this->on_write_data_request_received(reqPtr);
        });
    });
    m_writeDataObserver.start();

    m_readDataObserver.set([=](auto&& req) {
        auto reqPtr = std::make_shared<read_data_terminal::Request>(std::move(req));
        this->m_ios.post([=]() {
            this->on_read_data_request_received(reqPtr);
        });
    });
    m_readDataObserver.start();
}

void StorageInterface::on_write_data_request_received(write_data_request req)
{
    auto& value = req->message().value();
    auto variable = value.first();
    auto data = storage::StorageProvider::byte_array(value.second().begin(), value.second().end());

    YOGI_LOG_DEBUG(m_logger, "Updating variable " << variable << " for " << m_section << "...");
    m_storageProvider.async_write_data(m_section, variable, data, [=](bool success) {
        auto msg = m_writeDataTerminal.make_response_message();
        msg.set_value(success);
        req->try_respond(msg);
    });
}

void StorageInterface::on_read_data_request_received(read_data_request req)
{
    auto variable = req->message().value();

    YOGI_LOG_DEBUG(m_logger, "Reading variable " << variable << " for " << m_section << "...");
    m_storageProvider.async_read_data(m_section, variable, [=](bool success, auto data) {
        auto msg = m_readDataTerminal.make_response_message();
        decltype(msg)::Pair pair;
        msg.set_allocated_value(&pair);
        pair.set_first(success);
        pair.set_second(std::string(data.begin(), data.end()));
        req->try_respond(msg);
        msg.release_value();
    });
}

} // namespace storage
