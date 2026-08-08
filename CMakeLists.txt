cmake_minimum_required(VERSION 3.10)

project(PentaGame)

add_subdirectory(../../raylib ${CMAKE_CURRENT_BINARY_DIR}/raylib)

add_executable(${PROJECT_NAME} 
    src/main.c 
    src/rocketlaunch.h
    src/avalanche.h
) 
target_include_directories(${PROJECT_NAME} PRIVATE
    ../../raylib/src/external
)
target_link_libraries(${PROJECT_NAME}  
    PRIVATE raylib 
) 
