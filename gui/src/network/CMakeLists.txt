add_library(zappy_network STATIC
    client/GuiConnection.cpp
    poll/PollManager.cpp
    socket/Socket.cpp
    GuiNetworkManager.cpp
)

target_include_directories(zappy_network PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_network PRIVATE
    -W
    -Wall
    -Wextra
)
