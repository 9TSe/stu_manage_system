cmake_minimum_required(VERSION 3.0)
project(test)
set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) #头文件
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}) #生成路径
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp) #源文件
add_executable(client_app  ${SRC_LIST}) #编译
target_link_libraries(client_app) 
