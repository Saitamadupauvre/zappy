#pragma once

#include "Scene.hpp"
#include "audio/IAudioManager.hpp"
#include "graphic/IRenderer.hpp"
#include "graphic/IMeshFactory.hpp"
#include "graphic/ITextureLoader.hpp"
#include "logger/ContextLogger.hpp"
#include "world/World.hpp"
#include "scene/world/AnimationClock.hpp"
#include "scene/camera/CameraController.hpp"
#include "scene/TileMap.hpp"
#include "scene/layout/GridLayout.hpp"
#include "scene/layout/TorusLayout.hpp"
#include "scene/factory/PlayerEntityFactory.hpp"
#include "scene/factory/EggEntityFactory.hpp"
#include "scene/world/PlayerTileSystem.hpp"
#include "scene/world/WorldBuilder.hpp"
#include "scene/SceneHudManager.hpp"
#include "event/WorldEvent.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace zappy {

class WorldScene : public Scene
{
public:
    WorldScene(graphic::IRenderer& renderer, graphic::IMeshFactory& meshFactory,
               graphic::ITextureLoader& textureLoader,
               audio::IAudioManager& audioMgr);
    ~WorldScene() override = default;

    void update(const World& world, float dt) override;
    void render(graphic::IRenderer& renderer) override;
    void handleEvent(const event::Event& ev) override;
    void setSendLine(std::function<void(std::string)> fn);
    void setSetFps(std::function<void(int)> fn);
    void setOnFovChange(std::function<void(float)> fn);
    void setOnFpsOverlay(std::function<void(bool)> fn);
    void setOnFullscreen(std::function<void(bool)> fn);
    void setOnResolution(std::function<void(int,int)> fn);
    void setOnExitGame(std::function<void()> fn);
    void setOnBackToMenu(std::function<void()> fn);

private:
    void onWorldResized(const event::WorldResizedEvent& e);
    void onPlayerAdded(const event::PlayerAddedEvent& e);
    void onPlayerMoved(const event::PlayerMovedEvent& e);
    void onPlayerRemoved(const event::PlayerRemovedEvent& e);
    void onEggAdded(const event::EggAddedEvent& e);
    void onEggRemoved(const event::EggRemovedEvent& e);
    void onTeamAdded(const event::TeamAddedEvent& e);
    void onPlayerLevelChanged(const event::PlayerLevelChangedEvent& e);
    void onBroadcast(const event::PlayerBroadcastEvent& e);
    void onEntitySelected(const event::EntitySelectedEvent& e);
    void onFollowToggle();
    void onMapLayoutCycle();
    void onTileShadingToggle();

    void rebuildWorld();
    void repositionEggs();
    void spawnWave(graphic::Vector3f center, graphic::Color4b color);
    void spawnRituals(const event::IncantationStartEvent& e);
    void spawnExplosion(const event::IncantationEndEvent& e);
    void refreshTeamDetailIfOpen(uint32_t playerId);
    void onTeamSelected(const std::string& team);
    void refreshTeamStats(const std::string& team);
    void clearTeamSelection();

    static constexpr float SPACING           = 2.0f;
    static constexpr float MIN_MOVE_DURATION = 0.5f;

    static constexpr graphic::EntityID WAVE_BASE_ID      = 1u << 25;
    static constexpr graphic::EntityID RITUAL_BASE_ID    = 1u << 26;
    static constexpr graphic::EntityID EXPLOSION_BASE_ID = 1u << 27;

    enum class MapLayoutMode { Grid, Torus };

    int  _worldW        = 0;
    int  _worldH        = 0;
    bool _pendingResync = false;
    int  _tilesReceived = 0;
    int  _tilesTotal    = 0;
    std::vector<event::EggAddedEvent>          _pendingEggs;
    std::unordered_map<uint32_t, EggState>     _spawnedEggs;
    bool  _showTiles            = true;
    bool  _showFpsOverlay       = false;
    bool  _showIncantation      = true;
    bool  _showBroadcastCircle  = true;
    bool  _showEggHatchAnim     = true;
    bool  _showTeamColorTags    = true;
    bool  _showGrass            = true;
    float _lastDt               = 0.016f;

    AnimationClock  _clock;
    GridLayout      _gridLayout{SPACING};
    TorusLayout     _torusLayout{};
    MapLayoutMode   _layoutMode   = MapLayoutMode::Grid;
    IMapLayout*     _activeLayout = &_gridLayout;

    TileMap             _tileMap;
    PlayerTileSystem    _tileSystem{_entities, _tileMap, _clock};
    WorldBuilder        _worldBuilder{_entities, _tileMap};
    SceneHudManager     _hudMgr;

    PlayerEntityFactory _playerFactory;
    EggEntityFactory    _eggFactory;
    std::unordered_map<std::string, graphic::Color4b> _teamColors;

    uint32_t         _selectedPlayerId = 0;
    std::string      _selectedTeam;
    std::unordered_map<uint32_t, Resources> _playerInventories;
    CameraController _camCtrl{_camera};

    // Resource info HUD (standalone, not delegated to SceneHudManager)
    std::function<void(std::string)> _sendLine;

    graphic::SkyboxHandle _skyboxHandle{};
    bool                  _skyboxReady = false;
    float                 _skyboxTime  = 0.f;

    uint32_t _nextWaveSlot = 0;

    graphic::ITextureLoader* _textureLoader = nullptr;
    const World*             _worldPtr      = nullptr;
    audio::IAudioManager*    _audioMgr      = nullptr;
    ContextLogger _log{"WorldScene"};
};

} // namespace zappy
