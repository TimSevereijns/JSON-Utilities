#pragma once

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

#include "json_traits.h"

namespace detail
{
// Inspired by: https://quuxplusone.github.io/blog/2018/07/23/metafilter/

template <typename DataType, typename = void> struct stack_tuple;

// Base case
template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<!json_utils::traits::is_container<DataType>::value>::type>
{
    using type = std::tuple<>;
};

// Recursive case
template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<json_utils::traits::is_container<DataType>::value>::type>
{
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<DataType>>(),
        std::declval<typename stack_tuple<typename DataType::value_type>::type>()));
};

template <typename ContainerType> using stack_tuple_t = typename stack_tuple<ContainerType>::type;
} // namespace detail

template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>> class base_handler
{
    using Ch = typename EncodingType::Ch;

  public:
    virtual bool on_default()
    {
        return true;
    }

    virtual bool on_null()
    {
        return true;
    }

    virtual bool on_bool(bool value)
    {
        return true;
    }

    virtual bool on_int(int value)
    {
        return true;
    }

    virtual bool on_uint(unsigned int value)
    {
        return true;
    }

    virtual bool on_int_64(std::int64_t value)
    {
        return true;
    }

    virtual bool on_uint_64(std::uint64_t value)
    {
        return true;
    }

    virtual bool on_double(double value)
    {
        return true;
    }

    virtual bool on_raw_number(const Ch* value, rapidjson::SizeType length, bool should_copy)
    {
        return true;
    }

    virtual bool on_string(const Ch* value, rapidjson::SizeType length, bool should_copy)
    {
        return true;
    }

    virtual bool on_object_start()
    {
        return true;
    }

    virtual bool on_key(const Ch* value, rapidjson::SizeType length, bool should_copy)
    {
        return true;
    }

    virtual bool on_object_end(rapidjson::SizeType length)
    {
        return true;
    }

    virtual bool on_array_start()
    {
        return true;
    }

    virtual bool on_array_end(rapidjson::SizeType length)
    {
        return true;
    }
};

template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>> class array_handler : public base_handler<ContainerType>
{
    using Ch = typename EncodingType::Ch;

  public:
    bool on_default() override
    {
        return true;
    }

    bool on_null() override
    {
        return true;
    }

    bool on_bool(bool value) override
    {
        return true;
    }

    bool on_int(int value) override
    {
        return true;
    }

    bool on_uint(unsigned int value) override
    {
        return true;
    }

    bool on_int_64(std::int64_t value) override
    {
        return true;
    }

    bool on_uint_64(std::uint64_t value) override
    {
        return true;
    }

    bool on_double(double value) override
    {
        return true;
    }

    bool on_raw_number(const Ch* value, rapidjson::SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_string(const Ch* value, rapidjson::SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_object_start() override
    {
        return true;
    }

    bool on_key(const Ch* value, rapidjson::SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_object_end(rapidjson::SizeType length) override
    {
        return true;
    }

    bool on_array_start() override
    {
        return true;
    }

    bool on_array_end(rapidjson::SizeType length) override
    {
        return true;
    }
};

template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>> class object_handler : public base_handler<ContainerType>
{
    using Ch = typename EncodingType::Ch;

  public:
    bool on_default() override
    {
        return true;
    }

    bool on_null() override
    {
        return true;
    }

    bool on_bool(bool value) override
    {
        return true;
    }

    bool on_int(int value) override
    {
        return true;
    }

    bool on_uint(unsigned int value) override
    {
        return true;
    }

    bool on_int_64(std::int64_t value) override
    {
        return true;
    }

    bool on_uint_64(std::uint64_t value) override
    {
        return true;
    }

    bool on_double(double value) override
    {
        return true;
    }

    bool on_raw_number(const Ch* value, rapidjson::SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_string(const Ch* value, rapidjson::SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_object_start() override
    {
        return true;
    }

    bool on_key(const Ch* value, rapidjson::SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_object_end(rapidjson::SizeType length) override
    {
        return true;
    }

    bool on_array_start() override
    {
        return true;
    }

    bool on_array_end(rapidjson::SizeType length) override
    {
        return true;
    }
};

template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>>
class delegating_handler
    : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, delegating_handler<ContainerType>>
{
    using Ch = typename EncodingType::Ch;

  public:
    bool Default()
    {
        return true;
    }

    bool Null()
    {
        return true;
    }

    bool Bool(bool value)
    {
        return true;
    }

    bool Int(int value)
    {
        return true;
    }

    bool Uint(unsigned int value)
    {
        return true;
    }

    bool Int64(std::int64_t value)
    {
        return true;
    }

    bool Uint64(std::uint64_t value)
    {
        return true;
    }

    bool Double(double value)
    {
        return true;
    }

    bool RawNumber(const Ch* value, rapidjson::SizeType length, bool should_copy)
    {
        return true;
    }

    bool String(const Ch* value, rapidjson::SizeType length, bool should_copy)
    {
        return true;
    }

    bool StartObject()
    {
        return true;
    }

    bool Key(const Ch* value, rapidjson::SizeType length, bool should_copy)
    {
        return true;
    }

    bool EndObject(rapidjson::SizeType length)
    {
        return true;
    }

    bool StartArray()
    {
        return true;
    }

    bool EndArray(rapidjson::SizeType length)
    {
        return true;
    }

  private:
    auto& get_active_handler()
    {
        assert(!m_stack.empty());
        return m_stack.top();
    }

    // Given a templated container type, `stack_tuple_t` will decompose that type into a tuple where
    // each subsequent entry in the type will equal the `value_type` of the preceeding type.
    //
    // For instance, the following holds:
    //
    // static_assert(
    //     std::is_same<
    //         stack_tuple_t<std::vector<std::list<std::vector<int>>>>,
    //             std::tuple<
    //                 std::vector<std::list<std::vector<int>>>, std::list<std::vector<int>>,
    //                 std::vector<int>>>::value,
    //     "Composition failed.");
    detail::stack_tuple_t<ContainerType> m_stack;
    std::size_t m_index = 0;
};

template <typename ContainerType> void parse_json(const char* json, ContainerType& container)
{
    rapidjson::Reader reader;
    delegating_handler handler;
    rapidjson::StringStream ss(json);
    if (reader.Parse(ss, handler)) {
        messages.swap(handler.messages_); // Only change it if success.
    } else {
        rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
        size_t o = reader.GetErrorOffset();
        std::cout << "Error: " << rapidjson::GetParseError_En(e) << std::endl;
        std::cout << " at offset " << o << " near '" << std::string(json).substr(o, 10) << "...'"
                  << std::endl;
    }
}

namespace json_utils
{
namespace sax_deserializer
{
template <typename ContainerType> auto from_json(const std::string& json)
{
    ContainerType container;
    parse_json(json.data(), container);
}
} // namespace sax_deserializer
} // namespace json_utils