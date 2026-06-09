add_library(zappy_core STATIC
    GameEngine.cpp
    executor/CommandExecutor.cpp
    manager/input/InputManager.cpp
    manager/entity/EntityManager.cpp
)

target_include_directories(zappy_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_core PRIVATE
    -W
    -Wall
    -Wextra
)

target_link_libraries(zappy_core PUBLIC
    zappy_world
    zappy_parser
)
