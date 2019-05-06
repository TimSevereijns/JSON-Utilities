cmake_minimum_required(VERSION 3.8)

enable_language(CXX)

project(Json-Utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

include_directories(
    source
    third-party/catch2/single_include
    third-party/rapid-json/include)

add_executable(Json-Utils tests/unit_tests.cpp source/json_utils.h)

target_link_libraries(Json-Utils stdc++)

