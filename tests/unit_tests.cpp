#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <json_utils.h>

namespace unit_tests
{
    template <typename NumericType> void test_serialization_of_numerics()
    {
        constexpr auto max_positive = std::numeric_limits<NumericType>::max();
        constexpr auto min_negative = std::numeric_limits<NumericType>::min();

        const std::vector<NumericType> container = { max_positive, min_negative };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(
            json == "[" + std::to_string(max_positive) + "," + std::to_string(min_negative) + "]");
    }
} // namespace unit_tests

TEST_CASE("Trait Detection")
{
    SECTION("Container Has emplace_back(...)")
    {
        STATIC_REQUIRE(json_utils::traits::has_emplace_back<std::vector<int>>::value);
        STATIC_REQUIRE(json_utils::traits::has_emplace_back<std::list<int>>::value);

        STATIC_REQUIRE_FALSE(json_utils::traits::has_emplace_back<std::map<int, int>>::value);
        STATIC_REQUIRE_FALSE(json_utils::traits::has_emplace_back<std::set<int>>::value);

        STATIC_REQUIRE_FALSE(json_utils::traits::has_emplace_back<std::map<int, int>>::value);
        STATIC_REQUIRE_FALSE(json_utils::traits::has_emplace_back<std::set<int>>::value);
    }

    SECTION("Container Has emplace(...)")
    {
        STATIC_REQUIRE(json_utils::traits::has_emplace<std::map<int, int>>::value);
        STATIC_REQUIRE(json_utils::traits::has_emplace<std::set<int>>::value);
        STATIC_REQUIRE(json_utils::traits::has_emplace<std::queue<int>>::value);

        STATIC_REQUIRE_FALSE(json_utils::traits::has_emplace<std::vector<int>>::value);
        STATIC_REQUIRE_FALSE(json_utils::traits::has_emplace<std::list<int>>::value);
    }

    SECTION("Containers to be Treated as JSON Arrays")
    {
        STATIC_REQUIRE(json_utils::traits::treat_as_array<std::vector<int>>::value);
        STATIC_REQUIRE(json_utils::traits::treat_as_array<int[8]>::value);
    }

    SECTION("Containers to be Treated as JSON Objects")
    {
        STATIC_REQUIRE(json_utils::traits::treat_as_object<std::map<int, int>>::value);

        STATIC_REQUIRE(
            json_utils::traits::treat_as_object<std::unordered_map<std::string, double>>::value);
    }

    SECTION("Treat std::vector<std::pair<...>> is a JSON Object")
    {
        STATIC_REQUIRE(
            json_utils::traits::treat_as_object<std::vector<std::pair<std::string, int>>>::value);

        STATIC_REQUIRE(
            json_utils::traits::treat_as_object<std::vector<std::pair<std::wstring, int>>>::value);

        STATIC_REQUIRE(
            json_utils::traits::treat_as_object<std::vector<std::pair<double, char>>>::value);
    }

    SECTION("Strings are Special")
    {
        STATIC_REQUIRE_FALSE(json_utils::traits::treat_as_array<std::string>::value);
        STATIC_REQUIRE_FALSE(json_utils::traits::treat_as_array<std::wstring>::value);
        STATIC_REQUIRE_FALSE(json_utils::traits::treat_as_array<char[8]>::value);
        STATIC_REQUIRE_FALSE(json_utils::traits::treat_as_array<wchar_t[8]>::value);
    }
}

TEST_CASE("Serialization of std::vector<...>", "[Standard Containers]")
{
    SECTION("Empty Container")
    {
        const std::vector<int> container;
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "[]");
    }

    SECTION("Container of One Element")
    {
        const std::vector<int> container = { 1 };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "[1]");
    }

    SECTION("Container of Multiple Elements")
    {
        const std::vector<int> container = { 1, 2, 3, 4, 5 };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "[1,2,3,4,5]");
    }
}

TEST_CASE("Serialization of std::map<...>", "[Standard Containers]")
{
    SECTION("Empty Container")
    {
        const std::map<std::string, int> container;
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "{}");
    }

    SECTION("Container of One Element")
    {
        const std::map<std::string, int> container = {
            { "The meaning of life, the universe, and everything", 42 }
        };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"({"The meaning of life, the universe, and everything":42})");
    }

    SECTION("Container of Multiple Elements")
    {
        const std::unordered_map<std::string, int> container = { { "key_one", 1 },
                                                                 { "key_two", 2 },
                                                                 { "key_three", 3 } };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"({"key_three":3,"key_two":2,"key_one":1})");
    }
}

TEST_CASE("Serialization of JSON Value Types")
{
    SECTION("Array of `bool`")
    {
        const std::vector<bool> container = { false, true };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "[false,true]");
    }

    SECTION("Array of std::int32_t")
    {
        unit_tests::test_serialization_of_numerics<std::int32_t>();
    }

    SECTION("Array of std::uin32_t")
    {
        unit_tests::test_serialization_of_numerics<std::uint32_t>();
    }

    SECTION("Array of std::int64_t")
    {
        unit_tests::test_serialization_of_numerics<std::int64_t>();
    }

    SECTION("Array of std::uint64_t")
    {
        unit_tests::test_serialization_of_numerics<std::uint64_t>();
    }
}

#if __cplusplus >= 201703L // C++17
TEST_CASE("Serialization of C++17 Constructs")
{
    SECTION("Array of std::string_view")
    {
        constexpr std::string_view hello = "Hello";
        constexpr std::string_view world = "World";

        const std::vector<std::string_view> container = { hello, world };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Hello","World"])");
    }
}
#endif