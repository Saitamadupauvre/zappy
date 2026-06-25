#pragma once

#include "behavior/hud/PlayerTagProvider.hpp"
#include "entity/Entity.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/Types.hpp"
#include "world/WorldTypes.hpp"
#include <memory>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace zappy {

class EntityManager;

class PlayerEntityFactory
{
public:
    PlayerEntityFactory() = default;
    void init(graphic::IRenderer& renderer, graphic::IMeshFactory& factory);

    void spawn(EntityManager& em, const PlayerState& player,
               const graphic::Color4b& color);

    void assignTeamColor(const std::string& team,
                         std::unordered_map<std::string, graphic::Color4b>& colors) const;

    void setPlayerTeamColor(uint32_t id, graphic::Color4b color);
    void setShowTeamColor(bool v);
    void removePlayer(uint32_t id) { _tagProviders.erase(id); }

private:
    graphic::IRenderer* _renderer = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<PlayerTagProvider>> _tagProviders;
    bool _showTeamColor = false;

    static constexpr int   MAX_LEVEL    = 8;
    static constexpr float TILE_SPACING = 2.0f;
    static constexpr float MODEL_SCALE  = 0.65f;

    static constexpr const char* LEVEL_MODEL_PATHS[MAX_LEVEL] = {
        "assets/model/player_lv1.glb",
        "assets/model/player_lv2.glb",
        "assets/model/player_lv3.glb",
        "assets/model/player_lv4.glb",
        "assets/model/player_lv5.glb",
        "assets/model/player_lv6.glb",
        "assets/model/player_lv7.glb",
        "assets/model/player_lv8.glb",
    };

    std::array<graphic::ModelHandle, MAX_LEVEL> _levelModels{};

    // Per-level skin mesh indices (Base.* primitives that receive team color tint).
    // Index = level-1, values = Raylib mesh indices (primitive order in the GLB).
    static const std::array<std::vector<int>, MAX_LEVEL> SKIN_MESH_INDICES;
    // Corrects the model's authored facing (Euler degrees, X/Y/Z). Tune Y if the
    // player faces the wrong way; X/Z if it's exported lying down.
    static constexpr graphic::Vector3f MODEL_ROTATION_OFFSET = {0.0f, 0.0f, 0.0f};
    // Native model height (world units before scaling). The name tag floats at the
    // top of the scaled model: MODEL_HEIGHT * MODEL_SCALE above the player's feet.
    static constexpr float MODEL_HEIGHT = 2.0f;
    static constexpr float TAG_OFFSET_Y = MODEL_HEIGHT * MODEL_SCALE;

    static const graphic::Color4b TEAM_PALETTE[];
    static constexpr int          PALETTE_SIZE = 8;
};

} // namespace zappy
