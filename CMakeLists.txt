cmake_minimum_required(VERSION 3.10)

project(Warfront)

#find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} 
	external/FastNoiseLite.h
	src/main.c
)

target_link_libraries(${PROJECT_NAME} 
	raylib
)
include_directories(external)
add_subdirectory(external/raylib)

target_include_directories(${PROJECT_NAME}
	PUBLIC external/raylib/bin/raylib/include
)

target_link_directories(${PROJECT_NAME}
	PUBLIC external/raylib/src
)
