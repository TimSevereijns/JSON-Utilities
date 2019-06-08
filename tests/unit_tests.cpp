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
    constexpr auto min = std::numeric_limits<NumericType>::min();
    constexpr auto max = std::numeric_limits<NumericType>::max();

    const std::vector<NumericType> container = { min, max };
    const auto json = json_utils::serialize_to_json(container);

    REQUIRE(json == "[" + std::to_string(min) + "," + std::to_string(max) + "]");
}

template <typename NumericType> void test_array_deserialization_of_numerics()
{
    constexpr auto min = std::numeric_limits<NumericType>::min();
    constexpr auto max = std::numeric_limits<NumericType>::max();

    const std::vector<NumericType> source_container = { min, max };
    const auto json = json_utils::serialize_to_json(source_container);

    const auto deserialization = json_utils::deserialize_from_json<std::vector<NumericType>>(json);

    REQUIRE(source_container == deserialization);
}

template <typename NumericType> void test_object_deserialization_of_numerics()
{
    constexpr auto min = std::numeric_limits<NumericType>::min();
    constexpr auto max = std::numeric_limits<NumericType>::max();

    const std::map<std::string, NumericType> source_container = { { "min", min }, { "max", max } };

    const auto json = json_utils::serialize_to_json(source_container);

    const auto deserialization =
        json_utils::deserialize_from_json<std::map<std::string, NumericType>>(json);

    REQUIRE(source_container == deserialization);
}

namespace sample
{
class simple_widget
{
  public:
    simple_widget() = default;

    explicit simple_widget(std::string key_name) : m_key{ std::move(key_name) }
    {
    }

    std::string get_key() const noexcept
    {
        return m_key;
    }

    void set_key(std::string key_name)
    {
        m_key = std::move(key_name);
    }

    friend bool operator==(const simple_widget& lhs, const simple_widget& rhs) noexcept
    {
        return lhs.m_key == rhs.m_key;
    }

  private:
    std::string m_key;
};

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const sample::simple_widget& widget)
{
    writer.StartObject();

    writer.Key("Purpose");
    writer.String(widget.get_key().c_str());

    writer.EndObject();
}

void from_json(const rapidjson::Document& document, sample::simple_widget& widget)
{
    if (!document.IsObject()) {
        return;
    }

    const auto member_iterator = document.FindMember("Purpose");
    if (member_iterator == document.MemberEnd()) {
        return;
    }

    widget.set_key(member_iterator->value.GetString());
}

template <typename EncodingType, typename AllocatorType>
void from_json(
    const rapidjson::GenericMember<EncodingType, AllocatorType>& member,
    sample::simple_widget& widget)
{
    if (!member.value.IsObject()) {
        return;
    }

    const auto json_object = member.value.GetObject();
    const auto member_iterator = json_object.FindMember("Purpose");

    if (member_iterator == json_object.MemberEnd() || !member_iterator->value.IsString()) {
        return;
    }

    widget.set_key(member_iterator->value.GetString());
}

std::string to_narrow_json_key(const sample::simple_widget& widget) noexcept
{
    return widget.get_key();
}

class composite_widget
{
  public:
    composite_widget() = default;

    explicit composite_widget(std::string key_name) : m_inner_widget{ std::move(key_name) }
    {
    }

    void set_inner_widget(sample::simple_widget widget)
    {
        m_inner_widget = std::move(widget);
    }

    simple_widget get_inner_widget() const noexcept
    {
        return m_inner_widget;
    }

    friend bool operator==(const composite_widget& lhs, const composite_widget& rhs) noexcept
    {
        return lhs.m_inner_widget == rhs.m_inner_widget;
    }

  private:
    simple_widget m_inner_widget;
};

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const sample::composite_widget& widget)
{
    writer.StartObject();

    writer.Key("Inner Widget");

    using json_utils::serializer::to_json;
    to_json(writer, widget.get_inner_widget());

    writer.EndObject();
}

void from_json(const rapidjson::Document& document, sample::composite_widget& widget)
{
    if (!document.IsObject()) {
        return;
    }

    const auto member_iterator = document.FindMember("Inner Widget");
    if (member_iterator == document.MemberEnd()) {
        return;
    }

    sample::simple_widget simple_widget;

    using json_utils::serializer::to_json;
    from_json(*member_iterator, simple_widget);

    widget.set_inner_widget(std::move(simple_widget));
}

class heterogeneous_widget
{
  public:
    heterogeneous_widget()
    {
        m_data =
            std::vector<std::string>{ "Test String One", "Test String Two", "Test String Three" };
    }

    const std::vector<std::string>& get_data() const
    {
        return m_data;
    }

    void set_data(std::vector<std::string>&& data)
    {
        m_data = std::move(data);
    }

    const std::string& get_timestamp() const
    {
        return m_timestamp;
    }

    void set_timestamp(std::string timestamp)
    {
        m_timestamp = std::move(timestamp);
    }

    friend bool
    operator==(const heterogeneous_widget& lhs, const heterogeneous_widget& rhs) noexcept
    {
        return lhs.m_timestamp == rhs.m_timestamp && lhs.m_data == rhs.m_data;
    }

  private:
    std::string m_timestamp = "2019/05/29";
    std::vector<std::string> m_data;
};

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const sample::heterogeneous_widget& widget)
{
    writer.StartObject();
    writer.Key("Timestamp");
    writer.String(widget.get_timestamp().c_str());

    writer.Key("Data");

    using json_utils::serializer::to_json;
    to_json(writer, widget.get_data());

    writer.EndObject();
}

void from_json(const rapidjson::Document& document, sample::heterogeneous_widget& widget)
{
    if (!document.IsObject()) {
        return;
    }

    const auto timestamp_iterator = document.FindMember("Timestamp");
    if (timestamp_iterator == document.MemberEnd()) {
        return;
    }

    widget.set_timestamp(timestamp_iterator->value.GetString());

    const auto vector_iterator = document.FindMember("Data");
    if (vector_iterator == document.MemberEnd()) {
        return;
    }

    using json_utils::deserializer::from_json;

    std::vector<std::string> data;
    from_json(vector_iterator->value, data);

    widget.set_data(std::move(data));
}
} // namespace sample
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
    SECTION("Array of bool")
    {
        const std::vector<bool> container = { false, true };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == "[false,true]");
    }

    SECTION("Array of const char*")
    {
        const char* message_one = "Message One";
        const char* message_two = "Message Two";

        const std::vector<const char*> container = { message_one, message_two };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Message One","Message Two"])");
    }

    SECTION("Array of const char*, with nullptr")
    {
        const char* message_one = "Message One";
        const char* message_two = nullptr;

        const std::vector<const char*> container = { message_one, message_two };
        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Message One",null])");
    }
}

TEST_CASE("Serialization of Numeric Types into JSON Array")
{
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

TEST_CASE("Serialization of Numeric Types into JSON Object")
{
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

    SECTION("Array of std::optional<...>")
    {
        const std::vector<std::optional<int>> container = { std::optional<int>{ 101 },
                                                            std::optional<int>{ 202 } };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"([101,202])");
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

    SECTION("Using std::unique_ptr<...> Set to Null")
    {
        auto container = std::vector<std::unique_ptr<std::string>>();
        container.emplace_back(std::make_unique<std::string>("Test"));
        container.emplace_back(nullptr);

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Test",null])");
    }

    SECTION("Using std::shared_ptr<...>")
    {
        const auto container =
            std::vector<std::shared_ptr<std::string>>{ std::make_shared<std::string>("Hello"),
                                                       std::make_shared<std::string>("World") };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Hello","World"])");
    }

    SECTION("Using std::shared_ptr<...> Set to Null")
    {
        const auto container =
            std::vector<std::shared_ptr<std::string>>{ std::make_shared<std::string>("Test"),
                                                       nullptr };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Test",null])");
    }

    SECTION("Using std::weak_ptr<...>")
    {
        const auto pointer_one = std::make_shared<std::string>("Hello");
        const auto pointer_two = std::make_shared<std::string>("World");

        const auto container = std::vector<std::weak_ptr<std::string>>{ pointer_one, pointer_two };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Hello","World"])");
    }

    SECTION("Using std::weak_ptr<...> Set to Null")
    {
        const auto pointer_one = std::make_shared<std::string>("Test");

        const auto container = std::vector<std::weak_ptr<std::string>>{
            pointer_one, std::make_shared<std::string>("Short lived temporary")
        };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"(["Test",null])");
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

TEST_CASE("Serializing a Custom Type")
{
    SECTION("Custom Type as Key")
    {
        const std::vector<std::pair<sample::simple_widget, std::list<std::shared_ptr<std::string>>>>
            container = {
                { sample::simple_widget{ "Widget One" },
                  { std::make_unique<std::string>("1"), std::make_unique<std::string>("2"),
                    std::make_unique<std::string>("3"), std::make_unique<std::string>("4"),
                    std::make_unique<std::string>("5"), nullptr } },
                { sample::simple_widget{ "Widget Two" },
                  { std::make_unique<std::string>("5"), std::make_unique<std::string>("6"),
                    std::make_unique<std::string>("7"), std::make_unique<std::string>("8"),
                    std::make_unique<std::string>("9") } }
            };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(
            json ==
            R"({"Widget One":["1","2","3","4","5",null],"Widget Two":["5","6","7","8","9"]})");
    }

    SECTION("Custom Type with Nested Custom Type")
    {
        const std::vector<sample::composite_widget> container = { sample::composite_widget{
            "JSON Serialization" } };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"([{"Inner Widget":{"Purpose":"JSON Serialization"}}])");
    }

    SECTION("Custom Type as Value in std::vector<...>")
    {
        const std::vector<sample::simple_widget> container = { sample::simple_widget{
            "JSON Serialization" } };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"([{"Purpose":"JSON Serialization"}])");
    }

    SECTION("Custom Type as Value in std::vector<std::pair<...>>")
    {
        // @note This test relies on Argument-Dependent Lookup in the serialization logic.

        const std::vector<std::pair<std::string, sample::simple_widget>> container = {
            std::make_pair<std::string, sample::simple_widget>(
                "Widget", sample::simple_widget{ "JSON Serialization" })
        };

        const auto json = json_utils::serialize_to_json(container);

        REQUIRE(json == R"({"Widget":{"Purpose":"JSON Serialization"}})");
    }
}

TEST_CASE("Deserialization of JSON Array into Vector of Numerics")
{
    SECTION("Array of std::int32_t")
    {
        test_array_deserialization_of_numerics<std::int32_t>();
    }

    SECTION("Array of std::uin32_t")
    {
        test_array_deserialization_of_numerics<std::uint32_t>();
    }

    SECTION("Array of std::int64_t")
    {
        test_array_deserialization_of_numerics<std::int64_t>();
    }

    SECTION("Array of std::uint64_t")
    {
        test_array_deserialization_of_numerics<std::uint64_t>();
    }
}

TEST_CASE("Deserialization of JSON Object into Map of Numerics")
{
    SECTION("Array of std::int32_t")
    {
        test_object_deserialization_of_numerics<std::int32_t>();
    }

    SECTION("Array of std::uin32_t")
    {
        test_object_deserialization_of_numerics<std::uint32_t>();
    }

    SECTION("Array of std::int64_t")
    {
        test_object_deserialization_of_numerics<std::int64_t>();
    }

    SECTION("Array of std::uint64_t")
    {
        test_object_deserialization_of_numerics<std::uint64_t>();
    }
}

TEST_CASE("Deserialization into a std::vector<...>")
{
    SECTION("Empty JSON Array")
    {
        using container_type = std::vector<int>;

        const container_type source_container = {};
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Array with Single Element")
    {
        using container_type = std::vector<int>;

        const container_type source_container = { 1 };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Array with Numerous Elements")
    {
        using container_type = std::vector<int>;

        const container_type source_container = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Array of Doubles")
    {
        using container_type = std::vector<double>;

        const container_type source_container = { 1.1, 2.2, 3.3 };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Array of Strings")
    {
        using container_type = std::vector<std::string>;

        const container_type source_container = { "String One", "String Two" };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Array of Strings")
    {
        using container_type = std::vector<bool>;

        const container_type source_container = { false, true };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::vector<std::pair<...>>")
{
    SECTION("With a Single Entry")
    {
        using container_type = std::vector<std::pair<std::string, int>>;

        const container_type source_container = { { "Test", 1 } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("With Multiple Entries")
    {
        using container_type = std::vector<std::pair<std::string, int>>;

        const container_type source_container = { { "Key One", 1 }, { "Key Two", 99 } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::vector<std::vector<...>>")
{
    SECTION("With a Single Entry")
    {
        using container_type = std::vector<std::vector<int>>;

        const container_type source_container = { { 1, 2, 3, 4 } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("With Multiple Entries")
    {
        using container_type = std::vector<std::vector<int>>;

        const container_type source_container = { { 1, 2, 3, 4 }, { 5, 6, 7, 8 } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::map<...>")
{
    SECTION("Empty JSON Object")
    {
        using container_type = std::map<std::string, int>;

        const container_type source_container = {};
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object with Single Element")
    {
        using container_type = std::map<std::string, int>;

        const container_type source_container = { { "Key", 1 } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object with Numerous Elements")
    {
        using container_type = std::map<std::string, int>;

        const container_type source_container = { { "keyOne", 1 },
                                                  { "keyTwo", 2 },
                                                  { "keyThree", 3 } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object of Doubles")
    {
        using container_type = std::map<std::string, double>;

        const container_type source_container = { { "keyOne", 1.99 },
                                                  { "keyTwo", 2.67 },
                                                  { "keyThree", 3.123 } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object of Strings")
    {
        using container_type = std::map<std::string, std::string>;

        const container_type source_container = { { "keyOne", "1.99" },
                                                  { "keyTwo", "2.67" },
                                                  { "keyThree", "3.123" } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object of Booleans")
    {
        using container_type = std::map<std::string, bool>;

        const container_type source_container = { { "keyOne", false },
                                                  { "keyTwo", true },
                                                  { "keyThree", false } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::map<std::string, std::vector<...>>")
{
    SECTION("With a Single Entry")
    {
        using container_type = std::map<std::string, std::vector<int>>;

        const container_type source_container = { { "objectOne", { 1, 2, 3, 4 } } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("With Multiple Entries")
    {
        using container_type = std::map<std::string, std::vector<int>>;

        const container_type source_container = { { "objectOne", { 1, 2, 3, 4 } },
                                                  { "objectTwo", { 5, 6, 7, 8 } } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::map<std::string, std::map<std::string, int>>")
{
    SECTION("With a Single Entry")
    {
        using container_type = std::map<std::string, std::map<std::string, int>>;

        const container_type source_container = { { "objectOne", std::map<std::string, int>{
                                                                     { "subKey", 10 } } } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("With Multiple Entries")
    {
        using container_type = std::map<std::string, std::map<std::string, int>>;

        const container_type source_container = {
            { "objectOne", std::map<std::string, int>{ { "subKey", 10 } } },
            { "objectTwo", std::map<std::string, int>{ { "subKey", 20 } } }
        };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::list<...>")
{
    using container_type = std::list<std::string>;

    SECTION("Empty JSON Object")
    {
        const container_type source_container = {};
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object with Single Element")
    {
        const container_type source_container = { "Hello" };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object with Numerous Elements")
    {
        const container_type source_container = {
            "Hello, ", "World.", "This ", "is ", "a ", "test."
        };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

#if __cplusplus >= 201703L // C++17

TEST_CASE("Deserialization of C++17 Constructs")
{
    // using container_type = std::map<std::string, std::optional<int>>;

    // SECTION("Array of std::optional<...>")
    // {
    //     const container_type container = { { "keyOne", std::optional<int>{ 101 } },
    //                                        { "keyTwo", std::optional<int>{ 202 } } };

    //     const auto json = json_utils::serialize_to_json(container);
    //     const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

    //     REQUIRE(json == R"([101,202])");
    // }
}

#endif

TEST_CASE("Deserialization into a std::set<...>")
{
    using container_type = std::set<std::string>;

    SECTION("Empty JSON Object")
    {
        const container_type source_container = {};
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object with Single Element")
    {
        const container_type source_container = { "Hello" };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Object with Numerous Elements")
    {
        const container_type source_container = {
            "Hello, ", "World.", "This ", "is ", "a ", "test."
        };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization into a std::set<std::map<std::string, int>>")
{
    using container_type = std::set<std::map<std::string, int>>;

    SECTION("JSON Array with Single Element")
    {
        const container_type source_container = { std::map<std::string, int>{ { "value", 10 } } };
        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }

    SECTION("JSON Array with Multiple Elements")
    {
        const container_type source_container = { std::map<std::string, int>{ { "value", 10 } },
                                                  std::map<std::string, int>{ { "value", 20 } } };

        const auto json = json_utils::serialize_to_json(source_container);
        const auto resultant_container = json_utils::deserialize_from_json<container_type>(json);

        REQUIRE(source_container == resultant_container);
    }
}

TEST_CASE("Deserialization of Custom Type")
{
    SECTION("JSON Object to sample::widget")
    {
        const sample::simple_widget widget{ "JSON Demonstration" };
        const auto json = json_utils::serialize_to_json(widget);
        const auto deserialization = json_utils::deserialize_from_json<sample::simple_widget>(json);

        REQUIRE(widget == deserialization);
    }

    SECTION("JSON Object to sample::composite_widget")
    {
        const sample::composite_widget widget{ "JSON Demonstration" };
        const auto json = json_utils::serialize_to_json(widget);
        const auto deserialization =
            json_utils::deserialize_from_json<sample::composite_widget>(json);

        REQUIRE(widget == deserialization);
    }

    SECTION("JSON Object to sample::heterogeneous_widget")
    {
        const sample::heterogeneous_widget widget;
        const auto json = json_utils::serialize_to_json(widget);
        const auto deserialization =
            json_utils::deserialize_from_json<sample::heterogeneous_widget>(json);

        REQUIRE(widget == deserialization);
    }
}

TEST_CASE("Error Handling")
{
    SECTION("Malformed JSON")
    {
        const auto lambda = [] {
            const auto json = R"("missing":"brackets")";
            const auto deserialization =
                json_utils::deserialize_from_json<std::map<std::string, std::string>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Int, Got a Bool")
    {
        const std::vector<bool> source_container = { false, true };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization = json_utils::deserialize_from_json<std::vector<int>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an UInt, Got a Bool")
    {
        const std::vector<bool> source_container = { false, true };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::uint64_t>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected a String, Got a Bool")
    {
        const std::vector<bool> source_container = { false, true };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::string>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an UInt, Got a String")
    {
        const std::vector<std::string> source_container = { "Invalid", "Argument" };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::uint32_t>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Int, Got a Null")
    {
        const std::vector<std::shared_ptr<int>> source_container = { nullptr };

        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::string>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Double, Got an Object")
    {
        const std::vector<std::map<std::string, bool>> source_container = {
            std::map<std::string, bool>{ { "keyOne", false } },
            std::map<std::string, bool>{ { "keyTwo", false } }
        };

        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<double>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Nested Object, Got a Bool")
    {
        const std::map<std::string, bool> source_container = { { "keyOne", false },
                                                               { "keyTwo", false } };

        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization = json_utils::deserialize_from_json<
                std::map<std::string, std::map<std::string, bool>>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Nested Array, Got a Bool")
    {
        const std::map<std::string, bool> source_container = { { "keyOne", false },
                                                               { "keyTwo", false } };

        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::map<std::string, std::vector<bool>>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Double, Got an Array")
    {
        const std::vector<std::vector<bool>> source_container = { { false, false },
                                                                  { true, true } };

        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<double>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Array, Got a Bool")
    {
        const std::vector<bool> source_container = { true, false };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::vector<bool>>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Object, Got a Bool")
    {
        const std::vector<bool> source_container = { true, false };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::map<std::string, bool>>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Bool, Got a Int")
    {
        constexpr auto min = std::numeric_limits<std::uint32_t>::min();
        constexpr auto max = std::numeric_limits<std::uint32_t>::max();

        const std::vector<std::uint32_t> source_container = { min, max };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization = json_utils::deserialize_from_json<std::vector<bool>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Int, Got a UInt")
    {
        constexpr auto min = std::numeric_limits<std::uint32_t>::min();
        constexpr auto max = std::numeric_limits<std::uint32_t>::max();

        const std::vector<std::uint32_t> source_container = { min, max };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::int32_t>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }

    SECTION("Expected an Int64, Got a UInt64")
    {
        constexpr auto min = std::numeric_limits<std::uint64_t>::min();
        constexpr auto max = std::numeric_limits<std::uint64_t>::max();

        const std::vector<std::uint64_t> source_container = { min, max };
        const auto json = json_utils::serialize_to_json(source_container);

        const auto lambda = [&] {
            const auto deserialization =
                json_utils::deserialize_from_json<std::vector<std::int64_t>>(json);
        };

        REQUIRE_THROWS_AS(lambda(), std::invalid_argument);
    }
}
