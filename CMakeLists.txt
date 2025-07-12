cmake_minimum_required(VERSION 3.10)

project(Warfront)

#find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} 
	src/main.c
	src/block.h
	src/block.c	
	src/entity.h		
	src/entity.c		
	src/entitynode.h		
	src/entitynode.c	
	src/gamemaster.h		
	src/gamemaster.c		
	src/pathmanager.h	
	src/pathmanager.c		
	src/terrain.h	
	src/terrain.c
	src/bullet.h	
	src/bullet.c
	src/rllight.h
	src/worldobject.h
	src/worldobject.c
	src/dynamicarray.h
	src/dynamicarray.c
	src/octree.h
	src/octree.c
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