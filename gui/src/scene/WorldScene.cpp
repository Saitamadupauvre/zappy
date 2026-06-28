#include "WorldScene.hpp"
#include "locator/Locator.hpp"
#include "audio/IAudioManager.hpp"
#include "i18n/I18n.hpp"
#include "behavior/wave/WaveBroadcastBehavior.hpp"
#include "behavior/incantation/RitualCircleBehavior.hpp"
#include "behavior/incantation/ExplosionBehavior.hpp"
#include "behavior/drawable/mesh/GroundDrawableBehavior.hpp"
#include "event/Event.hpp"
#include "behavior/transform/TransformBehavior.hpp"
#include "behavior/movement/MovementBehavior.hpp"
#include "behavior/drawable/mesh/MeshDrawableBehavior.hpp"
#include "behavior/drawable/model/ModelDrawableBehavior.hpp"
#include "scene/builder/EntityBuilder.hpp"
#include "scene/builder/BuildersIncludes.hpp"
#include "scene/layout/IMapLayout.hpp"
#include "scene/factory/MapGroundFactory.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <vector>

namespace zappy {

WorldScene::WorldScene(graphic::IRenderer& renderer, graphic::IMeshFactory& meshFactory,
                       graphic::ITextureLoader& textureLoader,
                       audio::IAudioManager& audioMgr)
{
    _renderer      = &renderer;
    _textureLoader = &textureLoader;
    _audioMgr      = &audioMgr;
    _audioMgr->init();

    _playerFactory.init(renderer, meshFactory);
    _eggFactory.init(renderer, meshFactory);
    _worldBuilder.init(renderer, meshFactory);
    _tileSystem.setActiveLayout(_activeLayout);

    _inputManager.bindTriggerListener(InputAction::CYCLE_LAYOUT, [this]() {
        handleEvent(event::Event{event::WorldEvent{event::MapLayoutCycleEvent{}}});
    });
    _inputManager.bindTriggerListener(InputAction::TOGGLE_TILES, [this]() {
        handleEvent(event::Event{event::WorldEvent{event::TileShadingToggleEvent{}}});
    });
    _inputManager.bindTriggerListener(InputAction::FOLLOW_TOGGLE, [this]() {
        onFollowToggle();
    });
    _inputManager.bindTriggerListener(InputAction::TOGGLE_LEADERBOARD, [this]() {
        _hudMgr.onToggleLeaderboard();
    });
    _inputManager.bindTriggerListener(InputAction::TOGGLE_SETTINGS, [this]() {
        _hudMgr.onToggleSettings();
    });
    _inputManager.bindTriggerListener(InputAction::ESCAPE, [this]() {
        _hudMgr.onEscape();
    });

    _hudMgr.setup(_hud, renderer, textureLoader, _inputManager);

    _hudMgr.leaderboard().setOnWorldInfoClick([this]() {
        _hudMgr.onWorldInfo([this]() -> WorldInfoProvider::Stats {
            WorldInfoProvider::Stats s;
            if (!_worldPtr) return s;
            for (int y = 0; y < _worldH; ++y) {
                for (int x = 0; x < _worldW; ++x) {
                    const auto& r = _worldPtr->getTile(x, y).resources;
                    s.totalResources.food      += r.food;
                    s.totalResources.linemate  += r.linemate;
                    s.totalResources.deraumere += r.deraumere;
                    s.totalResources.sibur     += r.sibur;
                    s.totalResources.mendiane  += r.mendiane;
                    s.totalResources.phiras    += r.phiras;
                    s.totalResources.thystame  += r.thystame;
                }
            }
            for (const auto& [id, p] : _worldPtr->getPlayers()) {
                int lvl = std::clamp(p.level, 1, 8);
                s.playersPerLevel[lvl - 1]++;
                s.totalPlayers++;
            }
            return s;
        });
    });

    _hudMgr.setOnSelectTeamClick([this](const std::string& team) {
        onTeamSelected(team);
    });

    // Wire chat-click: open team chat for the currently selected player
    _hudMgr.playerInfo().setOnChatClick([this]() {
        auto team = _hudMgr.getPlayerTeam(_selectedPlayerId);
        if (!team.empty())
            _hudMgr.chat().open(_selectedPlayerId, team);
    });

    // Wire inventory-click: toggle inventory panel for selected player
    _hudMgr.playerInfo().setOnInventoryClick([this]() {
        _hudMgr.inventory().toggle();
    });

    // Wire follow-click from team detail panel: select player then start following
    _hudMgr.teamDetail().setOnFollowClick([this](uint32_t id) {
        handleEvent(event::Event{event::LogicEvent{event::EntitySelectedEvent{id}}});
        if (!_camera.isFollowing())
            _camCtrl.onFollowToggle(_selectedPlayerId, _entities, _hud);
    });

    _hudMgr.setOnSoundVolume([this](float v) { if (_audioMgr) _audioMgr->setSoundVolume(v); });
    _hudMgr.setOnMusicVolume([this](float v) { if (_audioMgr) _audioMgr->setMusicVolume(v); });

    _hudMgr.setOnIncantationEffect([this](bool v) { _showIncantation = v; });
    _hudMgr.setOnBroadcastCircle([this](bool v)  { _showBroadcastCircle = v; });
    _hudMgr.setOnEggHatchAnim([this](bool v)      { _showEggHatchAnim = v; });
    _hudMgr.setOnTeamColorTags([this](bool v) {
        _showTeamColorTags = v;
        _playerFactory.setShowTeamColor(v);
    });
    _hudMgr.setOnSkyMode([this](int idx) {
        static constexpr const char* SKY_SHADERS[] = { "skybox", "earth", "minimal_sky", "city", nullptr };
        _skyboxReady = (idx < 4);
        if (_skyboxReady)
            if (auto* r = zappy::Locator::getRenderer()) r->setSkyboxShader(_skyboxHandle, SKY_SHADERS[idx]);
    });
    _hudMgr.setOnGrass([this](bool v) {
        _showGrass = v;
        auto e = _entities.getEntity(MapGroundFactory::GROUND_ENTITY_ID);
        if (e) {
            auto gd = e->getBehavior<behavior::GroundDrawableBehavior>();
            if (gd) gd->setGrassEnabled(v);
        }
    });

    _skyboxHandle = renderer.uploadSkybox();
    _skyboxReady  = true;
}

void WorldScene::setSendLine(std::function<void(std::string)> fn)
{
    _sendLine = fn;
    _hudMgr.setSendLine([this](std::string s) {
        if (_sendLine) _sendLine(s);
        if (s.rfind("sst ", 0) == 0) {
            std::string tu = s.size() > 4 ? s.substr(4) : s;
            _hudMgr.pushPopup(i18n::tr(i18n::key::POPUP_SPEED_TITLE),
                              std::string(i18n::tr(i18n::key::POPUP_SPEED_SUB)) + ": " + tu + "t/s",
                              {100, 180, 255, 255});
        }
    });
}

void WorldScene::setSetFps(std::function<void(int)> fn)         { _hudMgr.setSetFps(std::move(fn)); }
void WorldScene::setOnFovChange(std::function<void(float)> fn)
{
    _hudMgr.setOnFovChange([this, fn = std::move(fn)](float v) {
        _camera.fov = v;
        if (fn) fn(v);
    });
}
void WorldScene::setOnFpsOverlay(std::function<void(bool)> fn)
{
    _hudMgr.setOnFpsOverlay([this, fn = std::move(fn)](bool v) {
        _showFpsOverlay = v;
        if (fn) fn(v);
    });
}
void WorldScene::setOnFullscreen(std::function<void(bool)> fn)  { _hudMgr.setOnFullscreen(std::move(fn)); }
void WorldScene::setOnResolution(std::function<void(int,int)> fn){ _hudMgr.setOnResolution(std::move(fn)); }
void WorldScene::setOnExitGame(std::function<void()> fn)         { _hudMgr.setOnExitGame(std::move(fn)); }
void WorldScene::setOnBackToMenu(std::function<void()> fn)
{
    _hudMgr.setOnEndScreenBackToMenu(fn);
    _hudMgr.setOnBackToMenu(std::move(fn));
}


void WorldScene::refreshTeamDetailIfOpen(uint32_t playerId)
{
    auto team = _hudMgr.getPlayerTeam(playerId);
    if (team.empty()) return;

    auto rawPlayers = _hudMgr.leaderboard().getPlayersForTeam(team);
    std::vector<PlayerDetailEntry> entries;
    entries.reserve(rawPlayers.size());
    for (const auto& ps : rawPlayers) {
        auto tile = _tileSystem.getTile(ps.id);
        entries.push_back({ps.id, ps.level, tile.first, tile.second});
    }
    _hudMgr.teamDetail().refreshIfOpen(team, std::move(entries));
}

void WorldScene::onWorldResized(const event::WorldResizedEvent& e)
{
    _log.info(std::format("World resized to {}x{}", e.width, e.height));
    _worldBuilder.clear(_worldW, _worldH);
    _worldW = e.width;
    _worldH = e.height;

    _tilesTotal    = _worldW * _worldH;
    _tilesReceived = 0;
    _hudMgr.showLoading();

    rebuildWorld();
    _tileSystem.repositionAll();
    _pendingResync = true;

    for (const auto& ev : _pendingEggs)
        onEggAdded(ev);
    _pendingEggs.clear();
}

void WorldScene::onFollowToggle()
{
    _camCtrl.onFollowToggle(_selectedPlayerId, _entities, _hud);
}

void WorldScene::onMapLayoutCycle()
{
    if (_layoutMode == MapLayoutMode::Grid) {
        _layoutMode   = MapLayoutMode::Torus;
        _activeLayout = &_torusLayout;
        _log.info("Map layout -> Torus");
    } else {
        _layoutMode   = MapLayoutMode::Grid;
        _activeLayout = &_gridLayout;
        _log.info("Map layout -> Grid");
    }
    _tileSystem.setActiveLayout(_activeLayout);

    if (_worldW <= 0 || _worldH <= 0) return;
    rebuildWorld();
    _tileSystem.repositionAll();
    repositionEggs();
    _pendingResync = true;
}

void WorldScene::onTileShadingToggle()
{
    _showTiles = !_showTiles;
    _log.info(_showTiles ? "Tile shading -> on" : "Tile shading -> off (Perlin only)");

    if (_worldW <= 0 || _worldH <= 0) return;
    _worldBuilder.rebuildGroundOnly(*_activeLayout, _worldW, _worldH, _showTiles);
}

void WorldScene::rebuildWorld()
{
    _worldBuilder.clear(_worldW, _worldH);
    _activeLayout->updateSizing(_worldW, _worldH);
    _camCtrl.applyLayoutFraming(*_activeLayout, _worldW, _worldH);
    _worldBuilder.build(*_activeLayout, _worldW, _worldH, _showTiles);

    auto e = _entities.getEntity(MapGroundFactory::GROUND_ENTITY_ID);
    if (e) {
        auto gd = e->getBehavior<behavior::GroundDrawableBehavior>();
        if (gd) gd->setGrassEnabled(_showGrass);
    }
}

void WorldScene::onPlayerAdded(const event::PlayerAddedEvent& e)
{
    const auto& p = e.player;
    _log.debug(std::format("Player {} added at ({},{}) team={}", p.id, p.x, p.y, p.team));

    graphic::Color4b color = graphic::Color4b::white();
    auto it = _teamColors.find(p.team);
    if (it != _teamColors.end()) color = it->second;

    // Remove any eggs at player's spawn tile (handles ebo arriving before GUI connected)
    std::vector<uint32_t> eggsToClear;
    for (const auto& [eggId, egg] : _spawnedEggs)
        if (egg.x == p.x && egg.y == p.y)
            eggsToClear.push_back(eggId);
    for (uint32_t eggId : eggsToClear) {
        _entities.removeEntity(EggEntityFactory::eggEntityId(eggId));
        _spawnedEggs.erase(eggId);
    }
    _pendingEggs.erase(
        std::remove_if(_pendingEggs.begin(), _pendingEggs.end(),
                       [&](const event::EggAddedEvent& ev){ return ev.egg.x == p.x && ev.egg.y == p.y; }),
        _pendingEggs.end());

    _playerFactory.spawn(_entities, p, color);
    _tileSystem.onPlayerAdded(p.id, p.x, p.y, p.orientation);
    _hudMgr.onPlayerAdded(p, color);
    _hudMgr.pushPopup(i18n::tr(i18n::key::POPUP_JOIN_TITLE),
                      std::string(i18n::tr(i18n::key::POPUP_JOIN_SUB)) + " " + std::to_string(p.id) + "  " + std::string(i18n::tr(i18n::key::TEAM)) + ": " + p.team,
                      {100, 220, 130, 255});
    refreshTeamDetailIfOpen(p.id);
}

void WorldScene::onPlayerMoved(const event::PlayerMovedEvent& e)
{
    _tileSystem.onPlayerMoved(e.id, e.x, e.y, e.orientation);
    _hudMgr.onPlayerMoved(e.id, _hudMgr.getPlayerTeam(e.id), e.x, e.y);
    refreshTeamDetailIfOpen(e.id);
    if (_audioMgr)
        _audioMgr->playAt(audio::SoundKind::WALK, _tileMap.standPos(e.x, e.y));
}

void WorldScene::onPlayerRemoved(const event::PlayerRemovedEvent& e)
{
    _log.debug(std::format("Player {} removed", e.id));
    _tileSystem.onPlayerRemoved(e.id);
    _entities.removeEntity(e.id);
    _hud.removeEntity(e.id + 10000);
    _playerInventories.erase(e.id);
    _playerFactory.removePlayer(e.id);
    _hudMgr.onPlayerRemoved(e.id, _selectedPlayerId);
    refreshTeamDetailIfOpen(e.id);
    _hudMgr.pushPopup(i18n::tr(i18n::key::POPUP_DIED_TITLE),
                      std::string(i18n::tr(i18n::key::POPUP_DIED_SUB)) + " " + std::to_string(e.id),
                      {220, 80, 80, 255});

    if (_selectedPlayerId == e.id) {
        _selectedPlayerId = 0;
        if (_camCtrl.isFirstPersonActive())
            _camCtrl.exitFirstPerson(0, _entities, _hud);
        else
            _camera.exitFollow();
    }
}

void WorldScene::onBroadcast(const event::PlayerBroadcastEvent& e)
{
    _hudMgr.onBroadcast(e.id, e.message);

    if (_tileSystem.hasTile(e.id)) {
        auto [tx, ty] = _tileSystem.getTile(e.id);
        auto center = _tileMap.standPos(tx, ty);
        graphic::Color4b color = graphic::Color4b::white();
        auto team = _hudMgr.getPlayerTeam(e.id);
        if (!team.empty()) {
            auto cit = _teamColors.find(team);
            if (cit != _teamColors.end()) color = cit->second;
        }
        spawnWave(center, color);
        if (_audioMgr)
            _audioMgr->playAt(audio::SoundKind::BROADCAST, center);
    }
}

void WorldScene::spawnWave(graphic::Vector3f center, graphic::Color4b color)
{
    if (!_showBroadcastCircle) return;
    static constexpr uint32_t WAVE_SLOTS      = 64;
    static constexpr float    WAVE_DURATION   = 2.5f;
    static constexpr float    WAVE_MAX_RADIUS = SPACING * 3.f;

    // Get ground mesh handle from the ground entity's drawable behavior
    graphic::MeshHandle groundMesh{};
    auto groundEntity = _entities.getEntity(MapGroundFactory::GROUND_ENTITY_ID);
    if (groundEntity) {
        auto gd = groundEntity->getBehavior<behavior::GroundDrawableBehavior>();
        if (gd) groundMesh = gd->getMeshHandle();
    }
    if (groundMesh.id == 0) return; // ground not ready

    graphic::EntityID id = WAVE_BASE_ID + (_nextWaveSlot % WAVE_SLOTS);
    _nextWaveSlot++;

    // Reuse slot: remove previous wave if it's still alive
    _entities.removeEntity(id);

    auto entity = _entities.createEntity(id, "wave");
    entity->addBehavior<behavior::WaveBroadcastBehavior>(
        _entities, id, groundMesh, center, color, WAVE_DURATION, WAVE_MAX_RADIUS);
}

void WorldScene::spawnRituals(const event::IncantationStartEvent& e)
{
    if (!_showIncantation) return;
    graphic::MeshHandle groundMesh{};
    auto groundEntity = _entities.getEntity(MapGroundFactory::GROUND_ENTITY_ID);
    if (groundEntity) {
        auto gd = groundEntity->getBehavior<behavior::GroundDrawableBehavior>();
        if (gd) groundMesh = gd->getMeshHandle();
    }
    if (groundMesh.id == 0) return;

    graphic::EntityID ritualId = RITUAL_BASE_ID + static_cast<uint32_t>(e.x) * 10000u + static_cast<uint32_t>(e.y);
    if (_entities.getEntity(ritualId)) return; // already active for this tile

    graphic::Vector3f center = _tileMap.standPos(e.x, e.y);
    graphic::Vector3f normal = _tileMap.tileUp(e.x, e.y);
    auto entity = _entities.createEntity(ritualId, "ritual");
    entity->addBehavior<behavior::RitualCircleBehavior>(
        _entities, ritualId, groundMesh, center, normal, SPACING, e.x, e.y);
    if (_audioMgr)
        _audioMgr->playAt(audio::SoundKind::INCANTATION, center);
}

void WorldScene::spawnExplosion(const event::IncantationEndEvent& e)
{
    if (_audioMgr)
        _audioMgr->stopSound(audio::SoundKind::INCANTATION);
    if (!e.success || !_showIncantation) return;

    static constexpr float DURATION   = 1.0f;
    static constexpr float MAX_RADIUS = SPACING * 1.4f;
    static constexpr float MAX_HEIGHT = 12.f;
    static constexpr int   NUM_FW     = 4;

    static constexpr graphic::Color4b COLORS[NUM_FW] = {
        { 255, 200,  50, 255 },
        { 255,  80,  80, 255 },
        {  80, 180, 255, 255 },
        { 200,  80, 255, 255 },
    };

    static constexpr float OX[NUM_FW] = {  0.f,  0.4f, -0.4f,  0.3f };
    static constexpr float OZ[NUM_FW] = {  0.f,  0.3f,  0.3f, -0.4f };

    graphic::Vector3f center = _tileMap.standPos(e.x, e.y);
    graphic::Vector3f normal = _tileMap.tileUp(e.x, e.y);

    graphic::Vector3f worldUp = { 0.f, 1.f, 0.f };
    graphic::Vector3f right = {
        normal.y * worldUp.z - normal.z * worldUp.y,
        normal.z * worldUp.x - normal.x * worldUp.z,
        normal.x * worldUp.y - normal.y * worldUp.x,
    };
    float rLen = std::sqrt(right.x*right.x + right.y*right.y + right.z*right.z);
    if (rLen < 0.001f) { right = { 1.f, 0.f, 0.f }; rLen = 1.f; }
    right.x /= rLen; right.y /= rLen; right.z /= rLen;
    graphic::Vector3f fwd = {
        normal.y * right.z - normal.z * right.y,
        normal.z * right.x - normal.x * right.z,
        normal.x * right.y - normal.y * right.x,
    };

    for (int i = 0; i < NUM_FW; i++) {
        graphic::Vector3f pos = {
            center.x + right.x * OX[i] + fwd.x * OZ[i],
            center.y + right.y * OX[i] + fwd.y * OZ[i],
            center.z + right.z * OX[i] + fwd.z * OZ[i],
        };
        graphic::EntityID id = EXPLOSION_BASE_ID
            + static_cast<uint32_t>(e.x) * 10000u
            + static_cast<uint32_t>(e.y)
            + static_cast<uint32_t>(i) * 1000000u;
        _entities.removeEntity(id);
        auto entity = _entities.createEntity(id, "explosion");
        entity->addBehavior<behavior::ExplosionBehavior>(
            _entities, id, pos, normal, COLORS[i],
            DURATION, MAX_RADIUS, MAX_HEIGHT, i * 0.15f);
        if (_audioMgr) {
            auto eb = entity->getBehavior<behavior::ExplosionBehavior>();
            if (eb) eb->setOnBurst([this, pos](graphic::Vector3f p) {
                _audioMgr->playAt(audio::SoundKind::EXPLOSION, p);
            });
        }
    }
}

void WorldScene::refreshTeamStats(const std::string& team)
{
    auto players = _hudMgr.leaderboard().getPlayersForTeam(team);
    std::vector<uint32_t> ids;
    ids.reserve(players.size());
    int maxLevel = 0;
    float avgLevel = 0.f;
    Resources total;
    for (const auto& ps : players) {
        ids.push_back(ps.id);
        if (ps.level > maxLevel) maxLevel = ps.level;
        avgLevel += static_cast<float>(ps.level);
        auto it = _playerInventories.find(ps.id);
        if (it != _playerInventories.end()) {
            total.food      += it->second.food;
            total.linemate  += it->second.linemate;
            total.deraumere += it->second.deraumere;
            total.sibur     += it->second.sibur;
            total.mendiane  += it->second.mendiane;
            total.phiras    += it->second.phiras;
            total.thystame  += it->second.thystame;
        }
    }
    if (!players.empty()) avgLevel /= static_cast<float>(players.size());

    _entities.handleEvent(event::Event{event::LogicEvent{
        event::TeamSelectEvent{std::move(ids), true}}});
    _hudMgr.onTeamSelected(team, static_cast<int>(players.size()), maxLevel, avgLevel, total);
}

void WorldScene::onTeamSelected(const std::string& team)
{
    if (_selectedTeam == team) {
        clearTeamSelection();
        return;
    }
    clearTeamSelection();
    _selectedTeam = team;

    _selectedPlayerId = 0;
    _hudMgr.clearSelectedPlayer();

    refreshTeamStats(team);
}

void WorldScene::clearTeamSelection()
{
    if (_selectedTeam.empty()) return;
    _selectedTeam.clear();
    _entities.handleEvent(event::Event{event::LogicEvent{
        event::TeamSelectEvent{{}, false}}});
    _hudMgr.onTeamDeselected();
}

void WorldScene::onEntitySelected(const event::EntitySelectedEvent& e)
{
    clearTeamSelection();

    auto team = _hudMgr.getPlayerTeam(e.entityId);
    if (team.empty()) {
        _selectedPlayerId = 0;
        if (_camera.isFollowing()) _camera.exitFollow();
        _hudMgr.clearSelectedPlayer();
        return;
    }

    bool wasFollowing = _camera.isFollowing();
    _selectedPlayerId = e.entityId;

    if (wasFollowing) {
        graphic::Vector3f pos{};
        auto entity = _entities.getEntity(e.entityId);
        if (entity) {
            auto tb = entity->getBehavior<behavior::TransformBehavior>();
            if (tb) pos = tb->getPosition();
            else if (_tileSystem.hasTile(e.entityId)) {
                auto [tx, ty] = _tileSystem.getTile(e.entityId);
                pos = _tileMap.standPos(tx, ty);
            }
        }
        _camera.enterFollow(pos);
    }

    int x = 0, y = 0;
    if (_tileSystem.hasTile(e.entityId)) {
        auto [tx, ty] = _tileSystem.getTile(e.entityId);
        x = tx; y = ty;
    }
    _hudMgr.onEntitySelected(e.entityId, team, x, y);
}

void WorldScene::onEggAdded(const event::EggAddedEvent& e)
{
    _log.debug(std::format("EGG ADD id={} x={} y={} tileMapBuilt={}",
        e.egg.id, e.egg.x, e.egg.y, _tileMap.built()));
    if (!_tileMap.built()) {
        _pendingEggs.push_back(e);
        return;
    }
    constexpr float EGG_RADIUS = 0.05f;
    auto up  = _tileMap.tileUp(e.egg.x, e.egg.y);
    auto pos = _tileMap.tilePos(e.egg.x, e.egg.y);
    pos.x += up.x * EGG_RADIUS;
    pos.y += up.y * EGG_RADIUS;
    pos.z += up.z * EGG_RADIUS;
    static constexpr graphic::Color4b EGG_DEFAULT_COLOR = {220, 220, 160, 255};
    auto colorIt = _teamColors.find(e.egg.team);
    graphic::Color4b eggColor = (colorIt != _teamColors.end()) ? colorIt->second : EGG_DEFAULT_COLOR;
    _eggFactory.spawn(_entities, e.egg.id, pos, eggColor);
    _spawnedEggs[e.egg.id] = e.egg;
}

void WorldScene::onEggRemoved(const event::EggRemovedEvent& e)
{
    _log.debug(std::format("EGG REMOVE id={} wasSpawned={} wasPending={}",
        e.id, _spawnedEggs.count(e.id) > 0,
        std::any_of(_pendingEggs.begin(), _pendingEggs.end(),
                    [&](const event::EggAddedEvent& ev){ return ev.egg.id == e.id; })));
    _entities.removeEntity(EggEntityFactory::eggEntityId(e.id));
    _spawnedEggs.erase(e.id);
    _pendingEggs.erase(
        std::remove_if(_pendingEggs.begin(), _pendingEggs.end(),
                       [&](const event::EggAddedEvent& ev){ return ev.egg.id == e.id; }),
        _pendingEggs.end());
}

void WorldScene::repositionEggs()
{
    constexpr float EGG_RADIUS = 0.05f;
    for (const auto& [id, egg] : _spawnedEggs) {
        auto entity = _entities.getEntity(EggEntityFactory::eggEntityId(id));
        if (!entity) continue;
        auto t = entity->getBehavior<behavior::TransformBehavior>();
        if (!t) continue;
        auto up  = _tileMap.tileUp(egg.x, egg.y);
        auto pos = _tileMap.tilePos(egg.x, egg.y);
        pos.x += up.x * EGG_RADIUS;
        pos.y += up.y * EGG_RADIUS;
        pos.z += up.z * EGG_RADIUS;
        t->setPosition(pos);
    }
}

void WorldScene::onTeamAdded(const event::TeamAddedEvent& e)
{
    _playerFactory.assignTeamColor(e.name, _teamColors);
    graphic::Color4b color = graphic::Color4b::white();
    auto it = _teamColors.find(e.name);
    if (it != _teamColors.end()) color = it->second;
    _hudMgr.onTeamAdded(e.name, color);
}

void WorldScene::onPlayerLevelChanged(const event::PlayerLevelChangedEvent& e)
{
    _hudMgr.onPlayerLevelChanged(e.id, e.level);
    refreshTeamDetailIfOpen(e.id);
    _hudMgr.pushPopup(i18n::tr(i18n::key::POPUP_LEVEL_TITLE),
                      std::string(i18n::tr(i18n::key::POPUP_JOIN_SUB)) + " " + std::to_string(e.id) + "  " + std::string(i18n::tr(i18n::key::POPUP_LEVEL_SUB)) + " " + std::to_string(e.level),
                      {255, 210, 60, 255});
}

void WorldScene::handleEvent(const event::Event& ev)
{
    event::on(ev,
        [&](const event::WorldResizedEvent& e)    { onWorldResized(e); },
        [&](const event::PlayerAddedEvent& e)     { onPlayerAdded(e); },
        [&](const event::PlayerMovedEvent& e)     { onPlayerMoved(e); },
        [&](const event::PlayerRemovedEvent& e)   { onPlayerRemoved(e); },
        [&](const event::EggAddedEvent& e)        { onEggAdded(e); },
        [&](const event::EggRemovedEvent& e)      { onEggRemoved(e); },
        [&](const event::TeamAddedEvent& e)            { onTeamAdded(e); },
        [&](const event::PlayerLevelChangedEvent& e)   { onPlayerLevelChanged(e); },
        [&](const event::IncantationStartEvent& e)  { spawnRituals(e); },
        [&](const event::IncantationEndEvent& e)    { spawnExplosion(e); },
        [&](const event::PlayerBroadcastEvent& e) { onBroadcast(e); },
        [&](const event::EntitySelectedEvent& e)  { onEntitySelected(e); },
        [&](const event::TimeUnitChangedEvent& e) {
            _clock.setTimeUnit(e.timeUnit);
            _hudMgr.setTimeUnit(e.timeUnit);
        },
        [&](const event::PlayerInventoryChangedEvent& e) {
            _playerInventories[e.id] = e.inventory;
            _hudMgr.onPlayerInventoryChanged(e.id, e.inventory);
            // Re-aggregate if this player belongs to the currently selected team
            if (!_selectedTeam.empty()) {
                auto team = _hudMgr.getPlayerTeam(e.id);
                if (team == _selectedTeam)
                    refreshTeamStats(_selectedTeam);
            }
        },
        [&](const event::TileChangedEvent&) {
            if (_tilesTotal > 0) {
                ++_tilesReceived;
                _hudMgr.onTileLoaded(_tilesReceived, _tilesTotal);
            }
        },
        [&](const event::ResourceCollectedEvent& e) {
            if (_audioMgr && _tileSystem.hasTile(e.playerId)) {
                auto [tx, ty] = _tileSystem.getTile(e.playerId);
                _audioMgr->playAt(audio::SoundKind::PICKUP, _tileMap.standPos(tx, ty));
            }
        },
        [&](const event::ResourceDroppedEvent& e) {
            if (_audioMgr && _tileSystem.hasTile(e.playerId)) {
                auto [tx, ty] = _tileSystem.getTile(e.playerId);
                _audioMgr->playAt(audio::SoundKind::PICKUP, _tileMap.standPos(tx, ty));
            }
        },
        [&](const event::MapLayoutCycleEvent&)    { onMapLayoutCycle(); },
        [&](const event::TileShadingToggleEvent&) { onTileShadingToggle(); },
        [&](const event::ServerUptimeEvent& e)    { _hudMgr.onServerUptime(e.uptimeSeconds); },
        [&](const event::GameEndedEvent& e)       {
            int playerCount = _worldPtr ? static_cast<int>(_worldPtr->getPlayers().size()) : 0;
            double elapsed  = _hudMgr.clock().getElapsedSeconds();
            _hudMgr.onGameEnded(e.winnerTeam, elapsed, playerCount,
                                _hudMgr.leaderboard().votedTeam());
        }
    );
    Scene::handleEvent(ev);
}

void WorldScene::update(const World& world, float dt)
{
    _worldPtr = &world;
    _lastDt = dt > 0.f ? dt : _lastDt;

    if (_audioMgr) {
        _audioMgr->setListenerPos(_camera.toCameraState().position);
        _audioMgr->update(dt);
    }

    _camera.update(dt, _inputManager);
    _camCtrl.update(dt, _selectedPlayerId, _entities);

    if (_camera.isFollowing() && _selectedPlayerId != 0) {
        auto entity = _entities.getEntity(_selectedPlayerId);
        bool hasTb  = entity && entity->getBehavior<behavior::TransformBehavior>();
        if (!hasTb && _tileSystem.hasTile(_selectedPlayerId)) {
            auto [tx, ty] = _tileSystem.getTile(_selectedPlayerId);
            _camera.updateFollowTarget(_tileMap.standPos(tx, ty), dt);
        }
    }

    if (_pendingResync && world.isReady()) {
        int total = 0;
        for (int y = 0; y < _worldH; ++y)
            for (int x = 0; x < _worldW; ++x) {
                const auto& r = world.getTile(x, y).resources;
                total += r.food + r.linemate + r.deraumere + r.sibur + r.mendiane + r.phiras + r.thystame;
                _entities.handleEvent(event::Event{event::WorldEvent{
                    event::TileChangedEvent{x, y, r}
                }});
            }
        _log.info(std::format("[RESYNC] {}x{} totalResources={}", _worldW, _worldH, total));
        _pendingResync = false;
    }

    _skyboxTime += dt;
    _entities.update(dt);
    _hudMgr.tick(dt);
}

void WorldScene::render(graphic::IRenderer& renderer)
{
    _renderer = &renderer;
    renderer.setCamera(_camera.toCameraState());

    graphic::Vector2f vp = renderer.getViewportSize();

    renderer.begin3D();
    if (_skyboxReady) renderer.drawSkybox(_skyboxHandle, _skyboxTime);
    _entities.handleEvent(event::RenderEvent{renderer, vp});
    renderer.end3D();

    renderer.begin2D();
    _hud.handleEvent(event::RenderEvent{renderer, vp});

    if (_showFpsOverlay) {
        int fps = _lastDt > 0.f ? static_cast<int>(1.f / _lastDt) : 0;
        graphic::TextStyle ts;
        ts.size  = 14.f;
        ts.color = fps >= 55 ? graphic::Color4b{80, 220, 100, 255}
                 : fps >= 25 ? graphic::Color4b{255, 200, 50, 255}
                             : graphic::Color4b{255, 80,  60, 255};
        renderer.drawText(std::to_string(fps) + " FPS", {8.f, 8.f}, ts);
    }

    renderer.end2D();
}

} // namespace zappy
