#include "FileWatcher.hh"

#include <sys/inotify.h>
#include <string.h>
#include <string>
#include <yogi.hpp>
using namespace std::string_literals;


FileWatcher::FileWatcher(boost::asio::io_service& ios)
: m_ios(ios)
, m_inotifySd(ios)
{
    m_inotifyFd = inotify_init();
    if (m_inotifyFd == -1) {
        throw std::runtime_error("Could not initialize inotify: "s + strerror(errno));
    }

    m_inotifySd.assign(m_inotifyFd);
    start_async_read_some();
}

FileWatcher::~FileWatcher()
{
    close(m_inotifyFd);
}

void FileWatcher::watch_file(const std::string& path, std::function<void ()> handlerFn)
{
    int wd = inotify_add_watch(m_inotifyFd, path.c_str(), IN_CLOSE_WRITE);
    if (wd == -1) {
        throw std::runtime_error("Could not watch "s + path + ": " + strerror(errno));
    };

    m_watches[wd].push_back([=] {
        YOGI_LOG_DEBUG("File change detected in " << path);
        handlerFn();
    });
}

void FileWatcher::start_async_read_some()
{
    m_inotifySd.async_read_some(boost::asio::buffer(m_buffer), [=](auto& ec, auto bytesRead) {
        if (!ec) {
            m_bufferStr.append(m_buffer.begin(), m_buffer.begin() + bytesRead);
            this->on_data_read();
        }
        else if (ec != boost::asio::error::operation_aborted) {
            YOGI_LOG_ERROR("Could not read inotify events: " << ec.message());
        }
    });
}

void FileWatcher::on_data_read()
{
    while (m_bufferStr.size() >= sizeof(inotify_event)) {
        auto* event = reinterpret_cast<const inotify_event*>(m_bufferStr.data());
        if (event->mask & IN_CLOSE_WRITE) {
            auto it = m_watches.find(event->wd);
            if (it != m_watches.end()) {
                for (auto& fn : it->second) {
                    fn();
                }
            }
        }

        m_bufferStr.erase(0, sizeof(inotify_event) + event->len);
    }

    start_async_read_some();
}
