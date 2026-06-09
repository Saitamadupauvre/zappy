add_library(zappy_scene STATIC
    Scene.cpp
    TestCubeScene.cpp
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
