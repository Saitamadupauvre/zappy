#include "PlayerEntityFactory.hpp"
#include "entity/Entity.hpp"
#include "core/manager/entity/EntityManager.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "behavior/movement/MovementBehavior.hpp"
#include "behavior/movement/RotationBehavior.hpp"
#include "behavior/animation/PlayerAnimationBehavior.hpp"
#include "behavior/player/BroadcastBehavior.hpp"
#include "behavior/player/PlayerLevelModelBehavior.hpp"
#include "behavior/clickable/ClickableBehavior.hpp"
#include "behavior/hoverable/HoverableBehavior.hpp"
#include "behavior/outline/OutlineBehavior.hpp"
#include "behavior/drawable/tag/TextDrawableBehavior.hpp"
#include "behavior/hud/PlayerTagProvider.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include "locator/Locator.hpp"
#include <algorithm>


namespace zappy {

const graphic::Color4b PlayerEntityFactory::TEAM_PALETTE[] = {
    { 50, 120, 255, 255},
    {255,  80,  50, 255},
    { 50, 200,  80, 255},
    {255, 200,  50, 255},
    {180,  50, 255, 255},
    { 50, 220, 220, 255},
    {255, 130,  30, 255},
    {220,  50, 130, 255},
};

// Skin mesh indices (Base.* primitives) per level derived from GLB structure.
// Each GLB has one mesh; Raylib splits primitives into separate meshes in order.
// Non-listed indices keep WHITE tint (wings=LeafMat, eyes, rings=none/Material).
const std::array<std::vector<int>, PlayerEntityFactory::MAX_LEVEL>
PlayerEntityFactory::SKIN_MESH_INDICES = {{
    {0},        // lv1: prim0=Base
    {0},        // lv2: prim0=Base
    {4},        // lv3: prim4=Base
    {1, 4},     // lv4: prim1=Base, prim4=Base
    {0, 2, 4},  // lv5: prim0=Base, prim2=Base, prim4=Base
    {0, 3},     // lv6: prim0=Base, prim3=Base
    {1, 3},     // lv7: prim1=Base, prim3=Base
    {1, 3},     // lv8: prim1=Base, prim3=Base
}};

void PlayerEntityFactory::init(graphic::IRenderer& renderer,
                                graphic::IMeshFactory& factory)
{
    (void)factory;
    _renderer = &renderer;
    for (int i = 0; i < MAX_LEVEL; ++i)
        _levelModels[i] = renderer.loadModel(LEVEL_MODEL_PATHS[i]);
}

void PlayerEntityFactory::spawn(EntityManager& em, const PlayerState& player, const graphic::Color4b& color) {
    const uint32_t playerId = player.id;
    const uint32_t tagId = playerId + 10000;

    auto& hud = zappy::Locator::getScene()->getHud();

    int  initLevel = std::clamp(player.level, 1, MAX_LEVEL);
    graphic::ModelHandle model = _levelModels[initLevel - 1];

    // Build initial per-mesh tints: skin meshes get team color, rest stay WHITE.
    auto buildInitialTints = [&](int levelIdx) {
        const auto& skinIndices = SKIN_MESH_INDICES[levelIdx];
        if (skinIndices.empty()) return std::vector<graphic::Color4b>{};
        int maxIdx = *std::max_element(skinIndices.begin(), skinIndices.end());
        std::vector<graphic::Color4b> tints(maxIdx + 1, {0, 0, 0, 0});
        for (int i : skinIndices)
            tints[i] = color;
        return tints;
    };

    auto playerEntity = EntityBuilder(em, playerId, "player")
        .transform().scale(MODEL_SCALE)
        .drawable().model(*_renderer, model, graphic::Color4b::white(), MODEL_ROTATION_OFFSET, false)
        .movement().animated(playerId)
        .player().state(player)
        .player().broadcast(playerId)
        .player().animation(*_renderer, model, playerId)
        .player().levelModel(*_renderer, _levelModels, playerId, player.level, color, SKIN_MESH_INDICES)
        .interaction().outline()
        .interaction().selectableOutline()
        .interaction().onClick([playerId]([[maybe_unused]] graphic::Entity& e) {})
        .tag().link(tagId, TAG_OFFSET_Y)
        .interaction().hoverScale(MODEL_SCALE, MODEL_SCALE + 0.2f)
        .build();

    // Apply initial skin tints now that the entity is built.
    if (auto drawable = playerEntity->getBehavior<behavior::ModelDrawableBehavior>())
        drawable->setMeshTints(buildInitialTints(initLevel - 1));

    auto statsProvider = std::make_shared<PlayerTagProvider>(playerId);
    statsProvider->setBroadcastBehavior(
        playerEntity->getBehavior<behavior::BroadcastBehavior>().get());
    statsProvider->setTeamColor(color);
    statsProvider->setShowTeamColor(_showTeamColor);
    _tagProviders[playerId] = statsProvider;

    EntityBuilder(hud, tagId, "player_tag")
        .hud().container(statsProvider)
        .hud().layout(behavior::hud::LayoutEngine::Type::Vertical, 5.0f)
        .hud().background(true, {255, 255, 255, 255}, {0, 0, 0, 255})
        .hud().autoSize()
        .hud().isWorldSpaceTag(true)
        .hud().hidden()
        .build();
}

void PlayerEntityFactory::setPlayerTeamColor(uint32_t id, graphic::Color4b color)
{
    auto it = _tagProviders.find(id);
    if (it != _tagProviders.end())
        it->second->setTeamColor(color);
}

void PlayerEntityFactory::setShowTeamColor(bool v)
{
    _showTeamColor = v;
    for (auto& [id, p] : _tagProviders)
        p->setShowTeamColor(v);
}

void PlayerEntityFactory::assignTeamColor(const std::string& team,
                                           std::unordered_map<std::string, graphic::Color4b>& colors) const
{
    if (colors.count(team)) return;
    colors[team] = TEAM_PALETTE[colors.size() % PALETTE_SIZE];
}

} // namespace zappy
