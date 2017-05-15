#ifndef FILE_WATCHER_HH
#define FILE_WATCHER_HH

#include <boost/asio/io_service.hpp>
#include <boost/array.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <regex>
#include <vector>
#include <unordered_map>
#include <memory>


class FileWatcher final
{
public:
    enum event_type_t {
        FILE_CREATED,
        FILE_DELETED,
        FILE_MODIFIED
    };

    typedef std::function<void (const std::string& filename, event_type_t)> handler_fn;

private:
    typedef int watch_descriptor;

    struct watch_data_t {
        std::string      directory;
        std::regex       filenamePattern;
        watch_descriptor directoryWd;
        handler_fn       handlerFn;
    };

    typedef std::shared_ptr<watch_data_t> watch_data_ptr;
    typedef std::unordered_map<watch_descriptor, std::vector<watch_data_ptr>> watches_map;

public:
    FileWatcher(boost::asio::io_service& ios);
    ~FileWatcher();

    void watch(const std::string& pattern, handler_fn handlerFn);

private:
    boost::asio::io_service&              m_ios;
    int                                   m_inotifyFd;
    boost::asio::posix::stream_descriptor m_inotifySd;
    boost::array<char, 4096>              m_buffer;
    std::string                           m_bufferStr;
    watches_map                           m_watches;

private:
    static std::string extract_directory(const std::string& pattern);
    static std::regex extract_filename_pattern(const std::string& pattern);
    static event_type_t maskToEventType(std::uint32_t mask);
    int add_watch(const std::string& path, bool isDir);
    void start_async_read_some();
    void on_data_read();
    void handle_event(const std::vector<watch_data_ptr>& dataVec, int wd, const std::string& path, event_type_t eventType);
    bool does_path_match_pattern(const std::string& path, const watch_data_ptr& data);
};

#endif // FILE_WATCHER_HH
