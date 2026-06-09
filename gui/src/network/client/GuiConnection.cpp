#include "GuiConnection.hpp"

namespace zappy {

GuiConnection::GuiConnection(GuiConnectionConfig config)
    : _config(std::move(config))
    , _socket(AF_INET, SOCK_STREAM)
{
}

void GuiConnection::connectToServer()
{
    auto address = Socket::makeAddress(_config.port, &_config.host);
    _socket.connect(address);
    _connected = true;
}

void GuiConnection::processReadable()
{
    if (_closed)
        return;

    try {
        const std::string chunk = _socket.read(4096);
        if (chunk.empty()) {
            _closed = true;
            return;
        }

        _incoming += chunk;
    } catch (const Socket::SocketException&) {
        _closed = true;
    }
}

void GuiConnection::processWritable()
{
    if (_closed)
        return;

    try {
        while (!_writeQueue.empty()) {
            _socket.writeAll(_writeQueue.front());
            _writeQueue.pop_front();
        }
    } catch (const Socket::SocketException&) {
        _closed = true;
    }
}

void GuiConnection::close()
{
    if (!_closed) {
        try {
            _socket.close();
        } catch (...) {
        }
        _closed = true;
        _connected = false;
    }
}

std::string GuiConnection::drainIncoming()
{
    std::string out;
    out.swap(_incoming);
    return out;
}

void GuiConnection::queueRaw(std::string data)
{
    if (!data.empty())
        _writeQueue.push_back(std::move(data));
}

void GuiConnection::queueLine(std::string line)
{
    if (!line.empty() && line.back() != '\n')
        line.push_back('\n');
    queueRaw(std::move(line));
}

} // namespace zappy