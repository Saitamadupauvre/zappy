add_library(zappy_behavior STATIC
    animation/PlayerAnimationBehavior.cpp
    drawable/ADrawable.cpp
    drawable/hud/InfoHudBehavior.cpp
    drawable/model/ModelDrawableBehavior.cpp
    drawable/mesh/MeshDrawableBehavior.cpp
    drawable/mesh/GroundDrawableBehavior.cpp
    drawable/tag/TextDrawableBehavior.cpp
    tag/TagBehavior.cpp
    egg/EggBehavior.cpp
    hoverable/HoverableBehavior.cpp
    hud/HudPicker.cpp
    hud/PlaceholderProvider.cpp
    hud/LayoutEngine.cpp
    incantation/IncantationTileBehavior.cpp
    incantation/RitualCircleBehavior.cpp
    incantation/ExplosionBehavior.cpp
    movement/MovementBehavior.cpp
    hud/HudContainerBehavior.cpp
    movement/RotationBehavior.cpp
    outline/OutlineBehavior.cpp
    pick/PickSystem.cpp
    player/BroadcastBehavior.cpp
    player/PlayerBehavior.cpp
    player/PlayerLevelModelBehavior.cpp
    wave/WaveBroadcastBehavior.cpp
    resource/ResourceBehavior.cpp
    selectable/SelectableBehavior.cpp
    transform/TransformBehavior.cpp
)

target_include_directories(zappy_behavior PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_behavior PRIVATE
    -W
    -Wall
    -Wextra
)
