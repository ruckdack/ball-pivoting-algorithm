cmake_minimum_required(VERSION 3.16)

project("Ball Pivoting Algorithm")

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)


file(GLOB_RECURSE SOURCE src/*)

add_executable(BPA ${SOURCE})
