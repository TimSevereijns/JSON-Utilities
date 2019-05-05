cmake_minimum_required(VERSION 3.13.2)

project(Json-Utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

include_directories(
    third-party/catch2/include
    third-party/rapid-json/include)

add_executable(Json-Utils source/main.cpp source/rapid-json-utils.h)

target_link_libraries(Json-Utils stdc++)

