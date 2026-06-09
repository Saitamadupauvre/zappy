#pragma once

#include "network/client/GuiConnection.hpp"
#include "network/poll/PollManager.hpp"
#include "network/GuiProtocol.hpp"
#include "parser/CommandParser/CommandParser.hpp"
#include "utils/Clock.hpp"

#include <string>
#include <deque>
#include <queue>

namespace zappy {

class GuiNetworkManager {
public:
    explicit GuiNetworkManager(GuiConnectionConfig config);
    ~GuiNetworkManager();

    void connect();
    void disconnect();
    void reconnect();
    void update(int pollTimeoutMs = 0);

    bool isConnected() const noexcept;
    bool isReady() const noexcept;

    bool tryPopCommand(net::Message& out);
    void sendLine(std::string line);

private:
    enum class HandshakeState { AwaitWelcome, AwaitServerInfo, Ready };

    void onReadable(int fd);
    void onWritable(int fd);
    void handleConnectionClosed(const std::string& reason);
    void processRaw(std::string raw);
    void handleLine(std::string line);

private:
    GuiConnectionConfig _config;
    GuiConnection       _connection;
    CommandParser       _parser;

    PollManager _poll;

    std::string _buffer;
    std::queue<net::Message> _messages;

    HandshakeState _hsState{HandshakeState::AwaitWelcome};
    int _serverInfoLines{0};
    bool _connected{false};
    bool _ready{false};
    bool _allowReconnect{true};

    utils::Clock _reconnectClock;
    int _reconnectIntervalMs{1000};
};

} // namespace zappy
