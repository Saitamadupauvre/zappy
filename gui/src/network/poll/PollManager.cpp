#include "PollManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>

void PollManager::addSocket(int fd, short events, Callback onReadable, Callback onWritable)
{
    struct pollfd pfd{};
    pfd.fd = fd;
    pfd.events = events;
    _entries.push_back({pfd, onReadable, onWritable});
}

void PollManager::updateSocket(int fd, short events)
{
    for (auto& entry : _entries) {
        if (entry.pfd.fd == fd) {
            entry.pfd.events = events;
            return;
        }
    }
}

void PollManager::removeSocket(int fd)
{
    auto it = std::find_if(_entries.begin(), _entries.end(),
        [fd](const auto& entry) { return entry.pfd.fd == fd; });

    if (it != _entries.end())
        _entries.erase(it);
}

void PollManager::pollLoop(int timeoutMs)
{
    if (_entries.empty()) return;

    std::vector<pollfd> pfds;
    pfds.reserve(_entries.size());
    for (auto& entry: _entries) {
        pfds.push_back(entry.pfd);
    }

    int ret = ::poll(pfds.data(), pfds.size(), timeoutMs);
    if (ret < 0)
        throw PollException("poll failed");

    for (size_t i = 0; i < pfds.size(); ++i) {
        // readCb may call removeSocket → _entries shrinks; guard before each access
        if ((pfds[i].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) && i < _entries.size() && _entries[i].readCb)
            _entries[i].readCb(pfds[i].fd);
        if ((pfds[i].revents & POLLOUT) && i < _entries.size() && _entries[i].writeCb)
            _entries[i].writeCb(pfds[i].fd);
    }
}
