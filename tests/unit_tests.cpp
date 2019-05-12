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

namespace
{
    template <typename NumericType> void test_serialization_of_numerics()
    {
        constexpr auto max = std::numeric_limits<NumericType>::max();
        constexpr auto min = std::numeric_limits<NumericType>::min();

        const std::vector<NumericType> container = { max, min };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "[" + std::to_string(max) + "," + std::to_string(min) + "]");
    }
} // namespace

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
        test_serialization_of_numerics<std::int32_t>();
    }

    SECTION("Array of std::uin32_t")
    {
        test_serialization_of_numerics<std::uint32_t>();
    }

    SECTION("Array of std::int64_t")
    {
        test_serialization_of_numerics<std::int64_t>();
    }

    SECTION("Array of std::uint64_t")
    {
        test_serialization_of_numerics<std::uint64_t>();
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

TEST_CASE("Handling Pointer Types")
{
    SECTION("Using std::unique_ptr<...>")
    {
        auto container = std::vector<std::unique_ptr<std::string>>();
        container.emplace_back(std::make_unique<std::string>("Hello"));
        container.emplace_back(std::make_unique<std::string>("World"));

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Hello","World"])");
    }

    SECTION("Using std::shared_ptr<...>")
    {
        const auto container =
            std::vector<std::shared_ptr<std::string>>{ std::make_shared<std::string>("Hello"),
                                                       std::make_shared<std::string>("World") };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Hello","World"])");
    }

    SECTION("Using std::weak_ptr<...>")
    {
        const auto pointer_one = std::make_shared<std::string>("Hello");
        const auto pointer_two = std::make_shared<std::string>("World");

        const auto container = std::vector<std::weak_ptr<std::string>>{ pointer_one, pointer_two };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Hello","World"])");
    }
}

TEST_CASE("Serializations of Composite Containrs")
{
    SECTION("std::map<std::string, std::vector<std::shared_ptr<std::string>>>")
    {
        const std::map<std::string, std::vector<std::shared_ptr<std::string>>> container = {
            { "Key One",
              std::vector<std::shared_ptr<std::string>>{
                  std::make_shared<std::string>("Value 1.A"),
                  std::make_shared<std::string>("Value 1.B"),
                  std::make_shared<std::string>("Value 1.C") } },
            { "Key Two",
              std::vector<std::shared_ptr<std::string>>{
                  std::make_shared<std::string>("Value 2.A"),
                  std::make_shared<std::string>("Value 2.B") } }
        };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(
            json ==
            R"({"Key One":["Value 1.A","Value 1.B","Value 1.C"],"Key Two":["Value 2.A","Value 2.B"]})");
    }

    SECTION("std::map<std::string, std::map<std::string, double>>")
    {
        const std::map<std::string, std::map<std::string, double>> container = {
            { "Key One", std::map<std::string, double>{ { "Subkey One", 16.0 },
                                                        { "Subkey Two", 32.0 },
                                                        { "Subkey Three", 64.0 } } },
            { "Key Two", std::map<std::string, double>{ { "Subkey One", 128.0 },
                                                        { "Subkey Two", 256.0 },
                                                        { "Subkey Three", 512.0 } } }
        };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(
            json ==
            R"({)"
            R"("Key One":{"Subkey One":16.0,"Subkey Three":64.0,"Subkey Two":32.0},)"
            R"("Key Two":{"Subkey One":128.0,"Subkey Three":512.0,"Subkey Two":256.0})"
            R"(})");
    }
}