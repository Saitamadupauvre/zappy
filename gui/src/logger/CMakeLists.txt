add_library(zappy_logger STATIC
    Logger.cpp
)

target_include_directories(zappy_logger PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_logger PRIVATE
    -W
    -Wall
    -Wextra
)
