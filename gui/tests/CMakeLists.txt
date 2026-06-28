cmake_minimum_required(VERSION 3.20)

find_library(CRITERION_LIB criterion REQUIRED)

set(COVERAGE_FLAGS --coverage -fprofile-arcs -ftest-coverage)

file(GLOB LOGGER_SOURCES ${PROJECT_SOURCE_DIR}/src/logger/*.cpp)

add_executable(unit_tests
    test_World.cpp
    test_CommandParser.cpp
    test_Resources.cpp
    test_TeamChatStore.cpp
    test_TeamLeaderboard.cpp
    test_Command.cpp
    test_CliParser.cpp
    test_Logger.cpp
    ${PROJECT_SOURCE_DIR}/src/world/World.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/CommandParser/CommandParser.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/Resources/Resources.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/command/Command.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/cli/CliParser.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/flag/Value/ValueFlag.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/flag/State/StateFlag.cpp
    ${PROJECT_SOURCE_DIR}/src/locator/Locator.cpp
    ${LOGGER_SOURCES}
)

target_include_directories(unit_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/parser
    ${PROJECT_SOURCE_DIR}/src/parser/cli
    ${PROJECT_SOURCE_DIR}/src/parser/command
    ${PROJECT_SOURCE_DIR}/src/parser/flag
    ${PROJECT_SOURCE_DIR}/src/parser/flag/Value
    ${PROJECT_SOURCE_DIR}/src/parser/flag/State
    ${PROJECT_SOURCE_DIR}/include/parser
    ${PROJECT_SOURCE_DIR}/src/logger
    ${PROJECT_SOURCE_DIR}/src/locator
    ${PROJECT_SOURCE_DIR}/src/world
)

target_compile_definitions(unit_tests PRIVATE
    DEFAULT_CONSOLE_LOG_LEVEL=zappy::LogLevel::NONE
    DEFAULT_FILE_LOG_LEVEL=zappy::LogLevel::NONE
    DEFAULT_LOG_FILE_PATH=""
)

target_compile_options(unit_tests PRIVATE
    -W -Wall -Wextra
    ${COVERAGE_FLAGS}
)

target_link_options(unit_tests PRIVATE ${COVERAGE_FLAGS})

target_link_libraries(unit_tests PRIVATE ${CRITERION_LIB})

set_target_properties(unit_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)
