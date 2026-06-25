#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace zappy {

class TeamChatStore {
public:
    struct Message {
        uint32_t    playerId;
        std::string text;
    };

    void setPlayerTeam(uint32_t playerId, const std::string& team) {
        _playerTeams[playerId] = team;
    }

    void removePlayer(uint32_t playerId) {
        _playerTeams.erase(playerId);
    }

    void addMessage(uint32_t playerId, const std::string& text) {
        auto it = _playerTeams.find(playerId);
        if (it == _playerTeams.end()) return;
        auto& msgs = _messages[it->second];
        if (msgs.size() >= MAX_MESSAGES)
            msgs.erase(msgs.begin());
        msgs.push_back({playerId, text});
        ++_version;
    }

    uint64_t getVersion() const { return _version; }

    const std::vector<Message>& getTeamMessages(const std::string& team) const {
        auto it = _messages.find(team);
        return (it != _messages.end()) ? it->second : _empty;
    }

    std::string getTeamForPlayer(uint32_t playerId) const {
        auto it = _playerTeams.find(playerId);
        return (it != _playerTeams.end()) ? it->second : "";
    }

private:
    static constexpr size_t MAX_MESSAGES = 200;
    inline static const std::vector<Message> _empty{};

    std::unordered_map<uint32_t, std::string>             _playerTeams;
    std::unordered_map<std::string, std::vector<Message>> _messages;
    uint64_t                                              _version = 1;
};

} // namespace zappy
