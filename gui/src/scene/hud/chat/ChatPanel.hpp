#pragma once

#include "behavior/hud/HudContainerBehavior.hpp"
#include "behavior/hud/TeamChatProvider.hpp"
#include "core/manager/hud/HudManager.hpp"
#include "graphic/Types.hpp"
#include "logger/ContextLogger.hpp"
#include "world/TeamChatStore.hpp"
#include <cstdint>
#include <memory>
#include <string>

namespace zappy {

class ChatPanel {
public:
    // Entity ID: 9997
    static constexpr graphic::EntityID CHAT_HUD_ID = 9997;

    void setup(HudManager& hud);

    void open(uint32_t playerId, const std::string& team);
    void close();
    void onBroadcast(uint32_t id, const std::string& message);
    void setPlayerTeam(uint32_t id, const std::string& team);
    void removePlayer(uint32_t id);

    bool isVisible() const;

private:
    TeamChatStore                                    _chatStore;
    std::shared_ptr<TeamChatProvider>               _provider;
    std::shared_ptr<behavior::HudContainerBehavior> _container;
    ContextLogger _log{"ChatPanel"};
};

} // namespace zappy
