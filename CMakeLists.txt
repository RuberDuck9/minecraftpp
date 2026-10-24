cmake_minimum_required(VERSION 3.20)

project(
	minecraftpp 
	VERSION 0.1
	LANGUAGES C CXX 
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(minecraftpp
	src/main.cpp

	include/glad/src/glad.c
)

target_compile_options(minecraftpp PRIVATE
	-Wall
	-Wextra
	-Werror
	-O3
	-g
)

target_link_libraries(minecraftpp PRIVATE
	OpenGL::OpenGL
	glfw
)

target_include_directories(minecraftpp PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include/glad/include
)
