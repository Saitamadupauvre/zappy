add_library(zappy_parser STATIC
    cli/CliParser.cpp
    flag/Value/ValueFlag.cpp
    flag/State/StateFlag.cpp
    CommandParser/CommandParser.cpp
    command/Command.cpp
    Resources/Resources.cpp
)

target_include_directories(zappy_parser PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/parser
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/parser
)

target_compile_options(zappy_parser PRIVATE
    -W
    -Wall
    -Wextra
)
