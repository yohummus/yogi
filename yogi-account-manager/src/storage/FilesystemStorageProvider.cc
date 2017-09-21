#include "FilesystemStorageProvider.hh"

#include <boost/filesystem.hpp>
#include <fstream>
using namespace std::string_literals;


namespace storage {

FilesystemStorageProvider::FilesystemStorageProvider(boost::asio::io_service& ios,
    const std::string& path)
: StorageProvider(ios)
, m_path(path)
, m_readDataError("Reading account or session data failed")
, m_writeDataError("Writing account or session data failed")
{
    create_root_directory();

    YOGI_LOG_INFO(logger(), "Storing data under " << m_path);
}

void FilesystemStorageProvider::async_clear(std::function<void (bool success)> completionHandler)
{
    using namespace boost::filesystem;

    bool success = true;
    for (directory_iterator it(m_path); it != directory_iterator(); ++it) {
        try {
            remove_all(it->path());
        }
        catch (const std::exception& e) {
            YOGI_LOG_ERROR(logger(), "Could not remove " << it->path() << ": " << e.what());
            success = false;
        }
    }

    io_service().post([=] { completionHandler(success); });
}

void FilesystemStorageProvider::async_clear_section(const std::string& section,
    std::function<void (bool success)> completionHandler)
{
    boost::filesystem::path dir = m_path / "...";

    bool success = true;
    try {
        security_check_paths(section, "*");
        dir = m_path / section;

        boost::filesystem::remove_all(dir);
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(logger(), "Could not delete " << dir << ": " << e.what());
        success = false;
    }

    io_service().post([=] { completionHandler(success); });
}

void FilesystemStorageProvider::async_read_data(const std::string& section,
    const std::string& variable,
    std::function<void (bool success, byte_array data)> completionHandler)
{
    boost::filesystem::path dir = m_path / "...";

    try {
        security_check_paths(section, variable);

        dir = m_path / section;
        auto file = dir / variable;

        byte_array data;
        if (boost::filesystem::exists(file)) {
            std::ifstream is(file.string(), std::ios::in | std::ios::binary);
            data.resize(boost::filesystem::file_size(file));
            YOGI_LOG_DEBUG(logger(), "Reading " << data.size() << " bytes from " << file << "...");
            is.read(data.data(), data.size());
        }
        else {
            YOGI_LOG_DEBUG(logger(), "File " << file << " does not exist. Responding with 0 bytes...");
        }

        io_service().post([=] { completionHandler(true, data); });
        m_readDataError.clear();
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(logger(), "Cannot read " << dir << ": " << e.what());
        io_service().post([=] { completionHandler(false, {}); });
        m_readDataError.set();
    }
}

void FilesystemStorageProvider::async_write_data(const std::string& section,
    const std::string& variable,
    const byte_array& data, std::function<void (bool success)> completionHandler)
{
    boost::filesystem::path dir;
    try {
        security_check_paths(section, variable);

        dir = m_path / section;
        auto file = dir / variable;

        boost::filesystem::create_directory(dir);

        if (data.empty()) {
            YOGI_LOG_DEBUG(logger(), "Deleting " << file << "...");
            boost::filesystem::remove(file);
        }
        else {
            YOGI_LOG_DEBUG(logger(), "Writing " << data.size() << " bytes to " << file << "...");
            boost::filesystem::create_directory(dir);
            std::ofstream os(file.string(), std::ios::out | std::ios::trunc | std::ios::binary);
            os.write(data.data(), data.size());
        }

        io_service().post([=] { completionHandler(true); });
        m_writeDataError.clear();
    }
    catch (const std::exception& e) {
        YOGI_LOG_ERROR(logger(), "Cannot write to " << dir << ": " << e.what());
        io_service().post([=] { completionHandler(false); });
        m_writeDataError.set();
    }
}

void FilesystemStorageProvider::create_root_directory()
{
    try {
        boost::filesystem::create_directories(m_path);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Could not create "s + m_path.string() + ": " + e.what());
    }
}

void FilesystemStorageProvider::security_check_paths(const std::string& section,
    const std::string& variable)
{
    for (auto& str : {section, variable}) {
        bool invalid = str.empty()
            || str == "."
            || str == ".."
            || (str.find("/") != std::string::npos)
            || (str.find("\\") != std::string::npos);

        if (invalid) {
            throw std::runtime_error("Invalid section or variable name");
        }
    }
}

} // namespace storage
