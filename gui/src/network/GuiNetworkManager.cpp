#include "GuiNetworkManager.hpp"

#include <iostream>

namespace zappy {

GuiNetworkManager::GuiNetworkManager(GuiConnectionConfig config)
    : _config(std::move(config))
    , _connection(_config)
{
}

GuiNetworkManager::~GuiNetworkManager()
{
    disconnect();
}

void GuiNetworkManager::connect()
{
    _allowReconnect = true;
    try {
        _connection.connectToServer();
        _connected = true;

        _poll.addSocket(_connection.fd(), POLLIN | POLLOUT,
            [this](int){ this->onReadable(_connection.fd()); },
            [this](int){ this->onWritable(_connection.fd()); }
        );
    } catch (const Socket::SocketException& e) {
        _connected = false;
        _reconnectClock.reset();
        std::cerr << "GuiNetworkManager: connect failed: " << e.what() << "\n";
    }
}

void GuiNetworkManager::update(int pollTimeoutMs)
{
    _reconnectClock.update();

    if (!_connected) {
        if (_allowReconnect && _reconnectClock.elapsedMsAtLeast(_reconnectIntervalMs)) {
            reconnect();
        }
        return;
    }

    _poll.pollLoop(pollTimeoutMs);
}

bool GuiNetworkManager::isConnected() const noexcept { return _connected; }
bool GuiNetworkManager::isReady() const noexcept { return _ready; }

bool GuiNetworkManager::tryPopCommand(net::Message& out)
{
    if (_messages.empty())
        return false;

    out = std::move(_messages.front());
    _messages.pop();
    return true;
}

void GuiNetworkManager::sendLine(std::string line)
{
    if (!line.empty() && line.back() != '\n')
        line.push_back('\n');
    _connection.queueRaw(std::move(line));
}

void GuiNetworkManager::onReadable(int)
{
    _connection.processReadable();
    const std::string raw = _connection.drainIncoming();
    if (!raw.empty())
        processRaw(raw);

    if (_connection.isClosed()) {
        handleConnectionClosed("connection closed by peer");
    }
}

void GuiNetworkManager::onWritable(int)
{
    _connection.processWritable();
    if (_connection.isClosed()) {
        handleConnectionClosed("connection closed during write");
    }
}

void GuiNetworkManager::handleConnectionClosed(const std::string& reason)
{
    _poll.removeSocket(_connection.fd());
    _connected = false;
    _ready = false;
    _allowReconnect = true;
    _reconnectClock.reset();
    std::cerr << "GuiNetworkManager: " << reason << "\n";
}

void GuiNetworkManager::processRaw(std::string raw)
{
    _buffer += std::move(raw);
    size_t pos = 0;
    while (true) {
        auto nl = _buffer.find('\n', pos);
        if (nl == std::string::npos) break;

        std::string line = _buffer.substr(0, nl);
        _buffer.erase(0, nl + 1);
        handleLine(std::move(line));
    }
}

void GuiNetworkManager::handleLine(std::string line)
{
    if (_hsState != HandshakeState::Ready) {
        if (_hsState == HandshakeState::AwaitWelcome) {
            if (line == "WELCOME") {
                _connection.queueLine(_config.teamName);
                _hsState = HandshakeState::AwaitServerInfo;
            }
            return;
        }

        if (_hsState == HandshakeState::AwaitServerInfo) {
            ++_serverInfoLines;
            if (_serverInfoLines >= 3) {
                _connection.queueLine("msz");
                _connection.queueLine("mct");
                _connection.queueLine("tna");
                _hsState = HandshakeState::Ready;
                _ready = true;
            }
            return;
        }
    }

    _messages.push(_parser.parseLine(line));
}

void GuiNetworkManager::disconnect()
{
    if (!_connected && _connection.isClosed()) return;
    _allowReconnect = false;
    _poll.removeSocket(_connection.fd());
    _connection.close();
    _connected = false;
    _ready = false;
}

void GuiNetworkManager::reconnect()
{
    if (_connected || !_allowReconnect) return;
    try {
        _connection.connectToServer();
        _poll.addSocket(_connection.fd(), POLLIN | POLLOUT,
            [this](int){ this->onReadable(_connection.fd()); },
            [this](int){ this->onWritable(_connection.fd()); }
        );
        _connected = true;
        _hsState = HandshakeState::AwaitWelcome;
        _serverInfoLines = 0;
        _ready = false;
        std::cerr << "GuiNetworkManager: reconnected\n";
    } catch (const Socket::SocketException& e) {
        _reconnectClock.reset();
        std::cerr << "GuiNetworkManager: reconnect failed: " << e.what() << "\n";
    }
}

} // namespace zappy
