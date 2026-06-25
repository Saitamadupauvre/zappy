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

    if(UNIX AND NOT APPLE)
        include(CheckIncludeFileCXX)
        check_include_file_cxx("wayland-client.h" HAVE_WAYLAND)
        if(HAVE_WAYLAND)
            set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
            set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
        else()
            set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
        endif()
    endif()

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
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
    RaylibShaderManager.cpp
)

target_link_libraries(zappy_graphics PUBLIC ${ZAPPY_RAYLIB_TARGET})

include(CheckStructHasMember)
get_target_property(_raylib_inc ${ZAPPY_RAYLIB_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
set(CMAKE_REQUIRED_INCLUDES "${_raylib_inc}")
check_struct_has_member("ModelAnimation" "frameCount" "raylib.h" RAYLIB_ANIM_HAS_FRAMECOUNT LANGUAGE CXX)
unset(CMAKE_REQUIRED_INCLUDES)

if(RAYLIB_ANIM_HAS_FRAMECOUNT)
    target_compile_definitions(zappy_graphics PRIVATE RAYLIB_ANIM_HAS_FRAMECOUNT)
endif()