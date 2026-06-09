add_library(zappy_graphics STATIC
    GraphicsContext.cpp
)

target_include_directories(zappy_graphics PUBLIC
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
)

target_compile_options(zappy_graphics PRIVATE
	-W
	-Wall
	-Wextra
)

add_subdirectory(raylib)
