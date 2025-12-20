cmake_minimum_required(VERSION 3.14)
project(Warden CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(Dependencies)
include(Formatting)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

include(Targets)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()