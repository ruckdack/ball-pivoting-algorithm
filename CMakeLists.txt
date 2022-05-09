cmake_minimum_required(VERSION 3.9)

project(bpa)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)


file(GLOB_RECURSE SOURCE src/*)

add_executable(BPA ${SOURCE})