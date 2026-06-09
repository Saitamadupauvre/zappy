include(FetchContent)

find_package(raylib CONFIG QUIET)

if(TARGET raylib)
    set(ZAPPY_RAYLIB_TARGET raylib)
else()
    find_package(PkgConfig QUIET)

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(RAYLIB QUIET IMPORTED_TARGET raylib)

        if(TARGET PkgConfig::RAYLIB)
            set(ZAPPY_RAYLIB_TARGET PkgConfig::RAYLIB)
        endif()
    endif()
endif()

if(NOT ZAPPY_RAYLIB_TARGET)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )
    FetchContent_MakeAvailable(raylib)

    set(ZAPPY_RAYLIB_TARGET raylib)
endif()

target_sources(zappy_graphics PRIVATE
    RaylibRenderer.cpp
    RaylibWindow.cpp
    RaylibMeshFactory.cpp
    RaylibTextureLoader.cpp
    RaylibFontLoader.cpp
)

target_link_libraries(zappy_graphics PUBLIC ${ZAPPY_RAYLIB_TARGET})