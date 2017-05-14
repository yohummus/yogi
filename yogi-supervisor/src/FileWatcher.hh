#ifndef FILE_WATCHER_HH
#define FILE_WATCHER_HH

#include <boost/asio/io_service.hpp>
#include <boost/array.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <map>
#include <vector>


class FileWatcher final
{
public:
    typedef int watch_id;
    typedef std::function<void ()> handler_fn;

public:
    FileWatcher(boost::asio::io_service& ios);
    ~FileWatcher();

    void watch_file(const std::string& path, handler_fn handlerFn);

private:
    boost::asio::io_service&                    m_ios;
    int                                         m_inotifyFd;
    boost::asio::posix::stream_descriptor       m_inotifySd;
    boost::array<char, 4096>                    m_buffer;
    std::string                                 m_bufferStr;
    std::map<watch_id, std::vector<handler_fn>> m_watches;

private:
    void start_async_read_some();
    void on_data_read();
};

#endif // FILE_WATCHER_HH
