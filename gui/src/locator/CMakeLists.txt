add_library(zappy_locator STATIC
    Locator.cpp
)

target_include_directories(zappy_locator PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_locator PRIVATE
    -W
    -Wall
    -Wextra
)
