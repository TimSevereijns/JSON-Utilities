cmake_minimum_required(VERSION 3.8)

enable_language(CXX)

project(Json-Utils)

set(CMAKE_CXX_STANDARD 17)

if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror -Wpedantic --coverage")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -Werror -Wpedantic")
endif (UNIX)

if (WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W4")
endif (WIN32)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(SOURCES
    tests/unit_tests.cpp
    source/future_std.h
    source/json_fwd.h
    source/json_traits.h
    source/json_serializer.h
    source/json_deserializer.h
    source/json_utils.h)

set(SOURCE_DIR
    source)

set(THIRD_PARTY
    third-party/catch2/single_include
    third-party/rapidjson/include)

include_directories(${SOURCE_DIR} ${THIRD_PARTY})

add_executable(Json-Utils ${SOURCES})

if (UNIX)
    target_link_libraries(Json-Utils stdc++)
endif (UNIX)
