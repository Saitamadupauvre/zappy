cmake_minimum_required(VERSION 3.20)

set(BINARY_NAME "zappy_gui")

project(
    ${BINARY_NAME}
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GUI_CONSOLE_LOG_LEVEL "zappy::LogLevel::NONE" CACHE STRING "Default console log level")
set(GUI_FILE_LOG_LEVEL "zappy::LogLevel::NONE" CACHE STRING "Default file log level")
set(GUI_LOG_FILE_PATH "\"\"" CACHE STRING "Default log file path")

add_compile_definitions(
    DEFAULT_CONSOLE_LOG_LEVEL=${GUI_CONSOLE_LOG_LEVEL}
    DEFAULT_FILE_LOG_LEVEL=${GUI_FILE_LOG_LEVEL}
    DEFAULT_LOG_FILE_PATH=${GUI_LOG_FILE_PATH}
)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps CACHE PATH "FetchContent base directory" FORCE)

# macOS: Homebrew's include/lib dirs are not on the default compiler search
# path. Expose them globally so targets that include <raylib.h> without linking
# the raylib target directly (e.g. zappy_core via RaylibWindow.hpp) compile.
if(APPLE)
    execute_process(COMMAND brew --prefix
        OUTPUT_VARIABLE BREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(BREW_PREFIX)
        include_directories(SYSTEM ${BREW_PREFIX}/include)
        link_directories(${BREW_PREFIX}/lib)
    endif()
endif()

add_subdirectory(src)

if(EXISTS ${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt)
    enable_testing()
    add_subdirectory(tests)
endif()
