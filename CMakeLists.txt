cmake_minimum_required(VERSION 3.16)

project(flopwise)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kuhn src/apps/kuhn_solve.cpp)
target_include_directories(kuhn PRIVATE src)
