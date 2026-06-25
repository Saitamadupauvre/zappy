#pragma once

#include "hud/IHudProvider.hpp"
#include "hud/HudElements.hpp"
#include "world/TeamChatStore.hpp"
#include <cstdint>
#include <string>

class TeamChatProvider : public behavior::hud::IHudProvider {
public:
    explicit TeamChatProvider(zappy::TeamChatStore* store) : _store(store) {}

    void setTeam(const std::string& team)    { _team = team; _lastStoreVersion = 0; }
    void setViewedPlayerId(uint32_t id)      { _viewedPlayerId = id; _lastStoreVersion = 0; }
    const std::string& getTeam() const       { return _team; }

    const std::vector<behavior::hud::HudElement>& getHudElements() const override {
        uint64_t storeV = _store ? _store->getVersion() : 0;
        if (storeV == _lastStoreVersion && !_cache.empty()) return _cache;
        _lastStoreVersion = storeV;
        ++_version;

        _cache.clear();
        if (!_store || _team.empty()) return _cache;

        auto& msgs = _store->getTeamMessages(_team);
        if (msgs.empty()) {
            _cache.push_back({ behavior::hud::TextData{"No messages yet.", 13.0f, {160, 160, 160, 200}} });
            return _cache;
        }

        _cache.reserve(msgs.size());
        for (auto& m : msgs) {
            bool isLeft = (m.playerId == _viewedPlayerId);
            graphic::Color4b color = isLeft
                ? graphic::Color4b{35,  80, 180, 220}
                : graphic::Color4b{40, 130,  70, 220};
            std::string sender = "Player " + std::to_string(m.playerId);
            _cache.push_back({ behavior::hud::ChatBubbleData{
                m.text, sender, isLeft, color, 240.0f, 13.0f
            }});
        }
        return _cache;
    }

    uint64_t getVersion() const override { return _version; }

private:
    zappy::TeamChatStore* _store          = nullptr;
    std::string           _team;
    uint32_t              _viewedPlayerId = 0;

    mutable std::vector<behavior::hud::HudElement> _cache;
    mutable uint64_t _version          = 1;
    mutable uint64_t _lastStoreVersion = 0;
};
