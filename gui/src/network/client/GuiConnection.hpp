#pragma once

#include "network/socket/Socket.hpp"

#include <cstdint>
#include <deque>
#include <string>

namespace zappy {

struct GuiConnectionConfig {
    std::string host{"127.0.0.1"};
    uint16_t port{0};
    std::string teamName{"GRAPHIC"};
};

class GuiConnection
{
public:
    explicit GuiConnection(GuiConnectionConfig config);

    void connectToServer();

    int fd() const noexcept { return _socket.fd(); }
    bool isConnected() const noexcept { return _connected; }
    bool wantsWrite() const noexcept { return !_writeQueue.empty(); }
    bool isClosed() const noexcept { return _closed; }

    void processReadable();
    void processWritable();
    void close();

    std::string drainIncoming();
    void queueRaw(std::string data);
    void queueLine(std::string line);

private:
    GuiConnectionConfig _config;
    Socket _socket;
    std::string _incoming;
    std::deque<std::string> _writeQueue;
    bool _connected{false};
    bool _closed{false};
};

} // namespace zappy