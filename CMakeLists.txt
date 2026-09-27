cmake_minimum_required(VERSION 3.11)
project(Warfront) 

add_subdirectory(../../raylib ${CMAKE_CURRENT_BINARY_DIR}/raylib)

add_executable(${PROJECT_NAME} 
    src/main.c 
    external/FastNoiseLite.h
) 
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ../../raylib/src/external
)

target_link_libraries(${PROJECT_NAME}  
    PRIVATE raylib 
)
