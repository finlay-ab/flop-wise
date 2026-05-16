cmake_minimum_required(VERSION 3.16)

project(flopwise LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kuhn 
    src/apps/kuhn_solve.cpp
    src/core/kuhn_game.cpp
    src/solver/cfr.cpp
    src/solver/infoset.cpp
)

target_include_directories(kuhn PRIVATE src)


add_executable(kuhn_tests
    tests/test_kuhn.cpp
    src/core/kuhn_game.cpp
    src/solver/cfr.cpp
    src/solver/infoset.cpp
)

target_include_directories(kuhn_tests PRIVATE src)

