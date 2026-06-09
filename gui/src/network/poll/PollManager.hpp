#pragma once
#include <vector>
#include <functional>
#include <poll.h>

#include <string>

class PollManager
{
public:
    using Callback = std::function<void(int fd)>;

    PollManager() = default;
    ~PollManager() = default;

    void addSocket(int fd, short events, Callback onReadable = nullptr, Callback onWritable = nullptr);
    void updateSocket(int fd, short events);
    void removeSocket(int fd);
    
    void pollLoop(int timeoutMs = -1);

    class PollException : public std::exception
    {
    public:
        PollException(const std::string& msg): _msg(msg) {}
        const char* what() const noexcept override { return _msg.c_str(); }
    private:
        std::string _msg;
    };

private:
    struct PollEntry {
        struct pollfd pfd;
        Callback readCb;
        Callback writeCb;
    };

    std::vector<PollEntry> _entries;
};
