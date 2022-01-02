cmake_minimum_required(VERSION 3.16)

project("Ball Pivoting Algorithm")

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE src/*)
list(REMOVE_ITEM SOURCE src/main.cpp)

add_executable(BPA src/main.cpp)
