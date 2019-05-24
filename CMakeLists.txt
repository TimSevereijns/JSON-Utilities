cmake_minimum_required(VERSION 3.8)

enable_language(CXX)

project(Json-Utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Werror -Wpedantic --coverage")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(SOURCES
    tests/unit_tests.cpp
    source/json_fwd.h
    source/json_traits.h
    source/json_serializer.h
    source/json_deserializer.h
    source/json_utils.h)

set(SOURCE_DIR
    source)

set(THIRD_PARTY
    third-party/catch2/single_include
    third-party/rapid-json/include)

include_directories(${SOURCE_DIR} ${THIRD_PARTY})

add_executable(Json-Utils ${SOURCES})

target_link_libraries(Json-Utils stdc++)
