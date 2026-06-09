add_library(zappy_world STATIC
    World.cpp
)

target_include_directories(zappy_world PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_world PRIVATE
    -W
    -Wall
    -Wextra
)
