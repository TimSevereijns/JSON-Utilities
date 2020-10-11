#pragma once

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "json_traits.h"

namespace detail
{
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

class token_handler
{
  public:
    virtual bool on_default()
    {
        return true;
    }

    virtual bool on_null()
    {
        return true;
    }

    virtual bool on_bool(bool /*value*/)
    {
        return true;
    }

    virtual bool on_int(int /*value*/)
    {
        return true;
    }

    virtual bool on_uint(unsigned int /*value*/)
    {
        return true;
    }

    virtual bool on_int_64(std::int64_t /*value*/)
    {
        return true;
    }

    virtual bool on_uint_64(std::uint64_t /*value*/)
    {
        return true;
    }

    virtual bool on_double(double /*value*/)
    {
        return true;
    }

    virtual bool
    on_raw_number(const char* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/)
    {
        return true;
    }

    virtual bool
    on_string(const char* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/)
    {
        return true;
    }

    virtual bool on_object_start()
    {
        return true;
    }

    virtual bool on_key(const char* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/)
    {
        return true;
    }

    virtual bool on_object_end(rapidjson::SizeType /*length*/)
    {
        return true;
    }

    virtual bool on_array_start()
    {
        return true;
    }

    virtual bool on_array_end(rapidjson::SizeType /*length*/)
    {
        return true;
    }
};

template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>>
class array_handler : public token_handler
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

    bool on_bool(bool /*value*/) override
    {
        return true;
    }

    bool on_int(int /*value*/) override
    {
        return true;
    }

    bool on_uint(unsigned int /*value*/) override
    {
        return true;
    }

    bool on_int_64(std::int64_t /*value*/) override
    {
        return true;
    }

    bool on_uint_64(std::uint64_t /*value*/) override
    {
        return true;
    }

    bool on_double(double /*value*/) override
    {
        return true;
    }

    bool on_raw_number(
        const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool
    on_string(const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool on_object_start() override
    {
        return true;
    }

    bool on_key(const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool on_object_end(rapidjson::SizeType /*length*/) override
    {
        return true;
    }

    bool on_array_start() override
    {
        return true;
    }

    bool on_array_end(rapidjson::SizeType /*length*/) override
    {
        return true;
    }

  private:
    ContainerType m_container;
};

template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>>
class object_handler : public token_handler
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

    bool on_bool(bool /*value*/) override
    {
        return true;
    }

    bool on_int(int /*value*/) override
    {
        return true;
    }

    bool on_uint(unsigned int /*value*/) override
    {
        return true;
    }

    bool on_int_64(std::int64_t /*value*/) override
    {
        return true;
    }

    bool on_uint_64(std::uint64_t /*value*/) override
    {
        return true;
    }

    bool on_double(double /*value*/) override
    {
        return true;
    }

    bool on_raw_number(
        const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool
    on_string(const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool on_object_start() override
    {
        return true;
    }

    bool on_key(const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool on_object_end(rapidjson::SizeType /*length*/) override
    {
        return true;
    }

    bool on_array_start() override
    {
        return true;
    }

    bool on_array_end(rapidjson::SizeType /*length*/) override
    {
        return true;
    }

  private:
    ContainerType m_container;
};

namespace detail
{
template <typename ContainerType>
auto make_token_handler() -> typename std::enable_if<
    json_utils::traits::treat_as_object_sink<ContainerType>::value,
    std::unique_ptr<token_handler>>::type
{
    return std::make_unique<object_handler<ContainerType>>();
}

template <typename ContainerType>
auto make_token_handler() -> typename std::enable_if<
    json_utils::traits::treat_as_array_sink<ContainerType>::value,
    std::unique_ptr<token_handler>>::type
{
    return std::make_unique<array_handler<ContainerType>>();
}

template <int... Is> struct indices
{
    using type = indices;
};

template <int N, int... Is> struct make_indices : make_indices<N - 1, N - 1, Is...>
{
};

template <int... Is> struct make_indices<0, Is...> : indices<Is...>
{
};

template <typename TupleType> class factory
{
    constexpr static int tuple_size = std::tuple_size<TupleType>::value;

  public:
    std::unique_ptr<token_handler> operator()(int index)
    {
        return produce(index);
    }

  private:
    std::unique_ptr<token_handler> produce(int index)
    {
        return produce_impl(make_indices<tuple_size>(), index);
    }

    template <int I, int... Is>
    std::unique_ptr<token_handler> produce_impl(indices<I, Is...>, int index)
    {
        if (I == index) {
            using container_type = typename std::tuple_element<I, TupleType>::type;
            return make_token_handler<container_type>();
        }

        return produce_impl(indices<Is...>(), index);
    }

    std::unique_ptr<token_handler> produce_impl(indices<>, int /*index*/)
    {
        throw "Uh-oh!";
    }
};
} // namespace detail

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

    bool Bool(bool /*value*/)
    {
        return true;
    }

    bool Int(int /*value*/)
    {
        return true;
    }

    bool Uint(unsigned int /*value*/)
    {
        return true;
    }

    bool Int64(std::int64_t /*value*/)
    {
        return true;
    }

    bool Uint64(std::uint64_t value)
    {
        return m_handlers[m_index]->on_uint_64(value);
    }

    bool Double(double value)
    {
        return m_handlers[m_index]->on_double(value);
    }

    bool RawNumber(const Ch* const value, rapidjson::SizeType length, bool should_copy)
    {
        return m_handlers[m_index]->on_raw_number(value, length, should_copy);
    }

    bool String(const Ch* const value, rapidjson::SizeType length, bool should_copy)
    {
        return m_handlers[m_index]->on_string(value, length, should_copy);
    }

    bool StartObject()
    {
        ++m_index;
        auto handler = m_handler_factory(m_index);
        m_handlers.emplace_back(std::move(handler));

        return true;
    }

    bool Key(const Ch* const /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/)
    {

        return true;
    }

    bool EndObject(rapidjson::SizeType /*length*/)
    {
        // @todo The handler needs to insert its finalized type into the outer container.
        // Perhaps this can be done via a static templatized method on the handler?

        --m_index;
        return true;
    }

    bool StartArray()
    {
        ++m_index;
        auto handler = m_handler_factory(m_index);
        m_handlers.emplace_back(std::move(handler));

        return true;
    }

    bool EndArray(rapidjson::SizeType /*length*/)
    {
        --m_index;
        return true;
    }

  private:
    // Given a templated container type, `stack_tuple_t` will decompose that type into a tuple where
    // each subsequent entry in the tuple will equal the `value_type` of the preceding type.
    //
    // For instance, the following assertion holds:
    //
    // static_assert(
    //     std::is_same<
    //         stack_tuple_t<std::vector<std::list<std::vector<int>>>>,
    //             std::tuple<
    //                 std::vector<std::list<std::vector<int>>>, std::list<std::vector<int>>,
    //                 std::vector<int>>>::value,
    //     "Decomposition failed.");
    using type_stack = detail::stack_tuple_t<ContainerType>;
    detail::factory<type_stack> m_handler_factory;

    std::vector<std::unique_ptr<token_handler>> m_handlers;

    std::int32_t m_index = -1;
};

template <typename ContainerType> void parse_json(const char* json, ContainerType& /*container*/)
{
    rapidjson::Reader reader;
    delegating_handler<ContainerType> handler;
    rapidjson::StringStream ss(json);
    if (reader.Parse(ss, handler)) {
        // messages.swap(handler.messages_); // Only change it if success.
    } else {
        rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
        std::size_t o = reader.GetErrorOffset();
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

    return container;
}
} // namespace sax_deserializer
} // namespace json_utils