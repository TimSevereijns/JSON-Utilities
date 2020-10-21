#pragma once

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "json_traits.h"

namespace json_utils
{
namespace detail
{
template <typename DataType, typename = void> struct stack_tuple;

// Base case
template <typename DataType>
struct stack_tuple<
    DataType,
    typename std::enable_if<!traits::treat_as_array_or_object_sink<DataType>::value>::type>
{
    using type = std::tuple<>;
};

// Recursive case
template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<traits::treat_as_array_or_object_sink<DataType>::value>::type>
{
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<DataType>>(),
        std::declval<typename stack_tuple<typename DataType::value_type>::type>()));
};

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
template <typename ContainerType> using stack_tuple_t = typename stack_tuple<ContainerType>::type;

template <typename Tuple> struct container_variant;

template <typename... Ts> struct container_variant<std::tuple<Ts...>>
{
    using type = std::variant<std::monostate, typename std::add_pointer<Ts>::type...>;
};

template <typename TupleType>
using container_variant_t = typename container_variant<TupleType>::type;

template <typename ContainerType, typename ElementType>
auto insert(ContainerType& container, ElementType&& element) ->
    typename std::enable_if<traits::has_emplace<ContainerType>::value>::type
{
    static_assert(
        std::is_convertible<
            typename std::decay<ElementType>::type, typename ContainerType::value_type>::value,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    container.emplace(std::forward<ElementType>(element));
}

template <typename ContainerType, typename ElementType>
auto insert(ContainerType& container, ElementType&& element) ->
    typename std::enable_if<traits::has_emplace_back<ContainerType>::value>::type
{
    static_assert(
        std::is_convertible<
            typename std::decay<ElementType>::type, typename ContainerType::value_type>::value,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    container.emplace_back(std::forward<ElementType>(element));
}

template <typename VariantType> class token_handler
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

    virtual VariantType get_container()
    {
        return { std::monostate{} };
    }
};

template <typename ContainerType, typename VariantType, typename EncodingType = rapidjson::UTF8<>>
class array_handler : public token_handler<VariantType>
{
    using Ch = typename EncodingType::Ch;

  public:
    bool on_default() override
    {
        return true;
    }

    bool on_null() override
    {
        // insert(m_container, nullptr);
        return true;
    }

    bool on_bool(bool value) override
    {
        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    bool on_int(int value) override
    {
        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    bool on_uint(unsigned int value) override
    {
        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    bool on_int_64(std::int64_t value) override
    {
        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    bool on_uint_64(std::uint64_t value) override
    {
        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    bool on_double(double value) override
    {
        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    bool on_raw_number(
        const Ch* /*value*/, rapidjson::SizeType /*length*/, bool /*should_copy*/) override
    {
        return true;
    }

    bool on_string(
        [[maybe_unused]] const Ch* value, [[maybe_unused]] rapidjson::SizeType length,
        bool /*should_copy*/) override
    {
        if constexpr (std::is_same_v<std::string, typename ContainerType::value_type>) {
            std::string data(value, length);
            insert(m_container, std::move(data));
        }

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

    VariantType get_container() override
    {
        return { &m_container };
    }

  private:
    ContainerType m_container;
};

template <typename ContainerType, typename VariantType, typename EncodingType = rapidjson::UTF8<>>
class object_handler : public token_handler<VariantType>
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
        finalize_pair_and_insert(value);
        return true;
    }

    bool on_int(int value) override
    {
        finalize_pair_and_insert(value);
        return true;
    }

    bool on_uint(unsigned int value) override
    {
        finalize_pair_and_insert(value);
        return true;
    }

    bool on_int_64(std::int64_t value) override
    {
        finalize_pair_and_insert(value);
        return true;
    }

    bool on_uint_64(std::uint64_t value) override
    {
        finalize_pair_and_insert(value);
        return true;
    }

    bool on_double(double value) override
    {
        finalize_pair_and_insert(value);
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

    bool on_key(const Ch* value, rapidjson::SizeType length, bool /*should_copy*/) override
    {
        m_key = std::make_unique<std::string>(value, length);
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

    VariantType get_container() override
    {
        return { &m_container };
    }

  private:
    template <typename DataType>
    void finalize_pair_and_insert(DataType&& value)
    {
        assert(m_key);

        if constexpr (std::is_convertible_v<decltype(value), typename decltype(m_value)::element_type>) {
            using value_type = typename decltype(m_value)::element_type;
            m_value = std::make_unique<value_type>(std::forward<DataType>(value));
            insert(m_container, std::make_pair(std::move(*m_key), std::move(*m_value)));
        }
    }

    static_assert(traits::is_pair<typename ContainerType::value_type>::value, "");

    std::unique_ptr<typename ContainerType::value_type::first_type> m_key;
    std::unique_ptr<typename ContainerType::value_type::second_type> m_value;

    ContainerType m_container;
};

template <typename ContainerType, typename VariantType>
auto make_token_handler() -> typename std::enable_if<
    traits::treat_as_object_sink<ContainerType>::value,
    std::unique_ptr<token_handler<VariantType>>>::type
{
    return std::make_unique<object_handler<ContainerType, VariantType>>();
}

template <typename ContainerType, typename VariantType>
auto make_token_handler() -> typename std::enable_if<
    traits::treat_as_array_sink<ContainerType>::value,
    std::unique_ptr<token_handler<VariantType>>>::type
{
    return std::make_unique<array_handler<ContainerType, VariantType>>();
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

template <typename TupleType, typename VariantType> class factory
{
    constexpr static int tuple_size = std::tuple_size<TupleType>::value;

  public:
    std::unique_ptr<token_handler<VariantType>> operator()(int index)
    {
        return produce(index);
    }

  private:
    std::unique_ptr<token_handler<VariantType>> produce(int index)
    {
        return produce_impl(make_indices<tuple_size>(), index);
    }

    template <int I, int... Is>
    std::unique_ptr<token_handler<VariantType>> produce_impl(indices<I, Is...>, int index)
    {
        if (I == index) {
            using container_type = typename std::tuple_element<I, TupleType>::type;
            return make_token_handler<container_type, VariantType>();
        }

        return produce_impl(indices<Is...>(), index);
    }

    std::unique_ptr<token_handler<VariantType>> produce_impl(indices<>, int /*index*/)
    {
        throw std::runtime_error("Out of range");
    }
};

template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
        return m_handlers[m_index]->on_null();
    }

    bool Bool(bool value)
    {
        return m_handlers[m_index]->on_bool(value);
    }

    bool Int(int value)
    {
        return m_handlers[m_index]->on_int(value);
    }

    bool Uint(unsigned int value)
    {
        return m_handlers[m_index]->on_uint(value);
    }

    bool Int64(std::int64_t value)
    {
        return m_handlers[m_index]->on_int_64(value);
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

    bool Key(const Ch* const value, rapidjson::SizeType length, bool should_copy)
    {
        return m_handlers[m_index]->on_key(value, length, should_copy);
    }

    bool EndObject(rapidjson::SizeType /*length*/)
    {
        return finalize_container();
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
        return finalize_container();
    }

    ContainerType* get_container()
    {
        return &m_container;
    }

  private:
    bool finalize_container()
    {
        assert(!m_handlers.empty());

        if (--m_index < 0) {
            auto variant = m_handlers.back()->get_container();
            m_container = *std::get<ContainerType*>(std::move(variant));
        } else {
            const auto source_variant = m_handlers[m_handlers.size() - 1]->get_container();
            const auto sink_variant = m_handlers[m_handlers.size() - 2]->get_container();

            const auto source_to_sink = [](auto* const source, auto* const sink) {
                assert(sink != nullptr);
                assert(source != nullptr);

                using sink_type = std::remove_pointer_t<decltype(sink)>;
                using source_type = std::remove_pointer_t<decltype(source)>;

                static_assert(traits::is_container<sink_type>::value, "Expected a container.");

                constexpr static bool is_insertable =
                    std::is_same_v<source_type, typename sink_type::value_type>;

                if constexpr (is_insertable) {
                    insert(*sink, std::move(*source));
                }
            };

            const auto source_handler = [&](auto* const source, auto& sink_variant) {
                std::visit(
                    overloaded{ [](const std::monostate&) {},
                                [&](auto* const sink) { source_to_sink(source, sink); } },
                    sink_variant);
            };

            std::visit(
                overloaded{ [](const std::monostate&) {},
                            [&](auto* const source) { source_handler(source, sink_variant); } },
                source_variant);
        }

        m_handlers.pop_back();

        --m_index;
        return true;
    }

    using type_stack = stack_tuple_t<ContainerType>;
    using container_variant = container_variant_t<type_stack>;
    factory<type_stack, container_variant> m_handler_factory;

    std::vector<std::unique_ptr<token_handler<container_variant>>> m_handlers;

    std::int32_t m_index = -1;

    ContainerType m_container;
};

template <typename ContainerType> void parse_json(const char* json, ContainerType& container)
{
    rapidjson::Reader reader;
    delegating_handler<ContainerType> handler;
    rapidjson::StringStream stream{ json };
    if (reader.Parse(stream, handler)) {
        container = std::move(*handler.get_container());
    } else {
        const auto errorCode = reader.GetParseErrorCode();
        const auto offset = reader.GetErrorOffset();

        auto message = std::string{ "Error: " } + rapidjson::GetParseError_En(errorCode) + "\n" +
                       " at offset " + std::to_string(offset) + "near '" +
                       std::string(json).substr(offset, 10) + "...'";

        throw std::runtime_error{ std::move(message) };
    }
}
} // namespace detail

namespace sax_deserializer
{
template <typename ContainerType> auto from_json(const std::string& json)
{
    ContainerType container;
    detail::parse_json(json.data(), container);

    return container;
}
} // namespace sax_deserializer
} // namespace json_utils