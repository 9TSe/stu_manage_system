cmake_minimum_required(VERSION 3.0)
project(test)
set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) #头文件
include_directories(/usr/include/mysql)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)  #源文件
add_executable(server_app  ${SRC_LIST})  #编译
target_link_libraries(server_app mysqlclient pthread) 
