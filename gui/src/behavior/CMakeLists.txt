add_library(zappy_behavior STATIC
    drawable/ADrawable.cpp
    drawable/mesh/MeshDrawableBehavior.cpp
    drawable/hud/HudInfoBehavior.cpp
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
