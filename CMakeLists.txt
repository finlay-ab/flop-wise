cmake_minimum_required(VERSION 3.16)
project(flopwise LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

add_library(flopwise_core
    src/core/kuhn_game.cpp
    src/solver/cfr.cpp
    src/solver/infoset.cpp
    src/solver/best_response.cpp   # you'll create this in Pass 2
    src/solver/strategy_book.cpp
)
target_include_directories(flopwise_core PUBLIC src)
target_compile_options(flopwise_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

add_executable(kuhn       src/apps/kuhn_solve.cpp)
add_executable(kuhn_tests tests/test_kuhn.cpp)
target_link_libraries(kuhn       PRIVATE flopwise_core)
target_link_libraries(kuhn_tests PRIVATE flopwise_core)

enable_testing()
add_test(NAME kuhn_tests COMMAND kuhn_tests)