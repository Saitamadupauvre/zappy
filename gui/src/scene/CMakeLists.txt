add_library(zappy_scene STATIC
    Scene.cpp
    WorldScene.cpp
    world/PlayerTileSystem.cpp
    world/WorldBuilder.cpp
    SceneHudManager.cpp
    hud/popup/PopupPanel.cpp
    hud/leaderboard/LeaderboardPanel.cpp
    hud/leaderboard/TeamDetailPanel.cpp
    hud/leaderboard/WorldInfoPanel.cpp
    hud/chat/ChatPanel.cpp
    hud/player/PlayerInfoPanel.cpp
    hud/resource/ResourceInfoPanel.cpp
    hud/speed/SpeedPanel.cpp
    hud/clock/ClockPanel.cpp
    hud/settings/SettingsPanel.cpp
    hud/inventory/InventoryPanel.cpp
    hud/teamstats/TeamStatsPanel.cpp
    hud/settings/KeybindingsSection.cpp
    hud/settings/VideoSection.cpp
    hud/settings/GameDisplaySection.cpp
    camera/OrbitCamera.cpp
    camera/CameraController.cpp
    factory/ResourceEntityFactory.cpp
    factory/PlayerEntityFactory.cpp
    factory/EggEntityFactory.cpp
    factory/MapGroundFactory.cpp
    grass/GrassBuilder.cpp
    builder/EntityBuilder.cpp
    builder/sub_builders/transform/TransformBuilder.cpp
    builder/sub_builders/movement/MovementBuilder.cpp
    builder/sub_builders/interaction/InteractionBuilder.cpp
    builder/sub_builders/drawable/DrawableBuilder.cpp
    builder/sub_builders/player/PlayerBuilder.cpp
    builder/sub_builders/resource/ResourceBuilder.cpp
    builder/sub_builders/egg/EggBuilder.cpp
    builder/sub_builders/tag/TagBuilder.cpp
    builder/sub_builders/hud/HudBuilder.cpp
)

target_include_directories(zappy_scene PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(zappy_scene PRIVATE
    zappy_behavior
    zappy_graphics
)

target_compile_options(zappy_scene PRIVATE
    -W
    -Wall
    -Wextra
)
