add_subdirectory(behavior)
add_subdirectory(scene)
add_subdirectory(world)
add_subdirectory(core)
add_subdirectory(network)
add_subdirectory(parser)
add_subdirectory(locator)
add_subdirectory(logger)
add_subdirectory(graphics)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -W
    -Wall
    -Wextra
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -Wl,--start-group
    zappy_core
    zappy_scene
    zappy_behavior
    -Wl,--end-group
    zappy_world
    zappy_network
    zappy_parser
    zappy_locator
    zappy_logger
    zappy_graphics
)
