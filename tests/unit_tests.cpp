#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include <json_utils.h>

TEST_CASE("Serialization of std::vector<...>", "[Standard Containers]")
{
    SECTION("Integers")
    {
        const std::vector<int> container;
        const auto json = json_utils::serialize_to_pretty_json(container);
    }
}