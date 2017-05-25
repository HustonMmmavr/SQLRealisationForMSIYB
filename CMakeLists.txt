cmake_minimum_required(VERSION 3.7)
project(untitled1)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp sqlite3.c )

set(CMAKE_CXX_FLAGS " -pthread -ldl -std=c++11 ")
add_executable(untitled1 ${SOURCE_FILES})
target_link_libraries(untitled1 ${CMAKE_DL_LIBS})