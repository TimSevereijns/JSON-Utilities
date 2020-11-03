#pragma once

#if __cplusplus >= 201703L // C++17

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

template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<
                  !traits::treat_as_array_or_object_sink<DataType>::value &&
                  !traits::is_pair<DataType>::value>::type>
{
    using type = std::tuple<>;
};

template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<
                  traits::is_pair<DataType>::value &&
                  traits::is_container<typename DataType::second_type>::value>::type>
{
    using type = typename stack_tuple<typename DataType::second_type>::type;
};

template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<
                  traits::is_pair<DataType>::value &&
                  !traits::is_container<typename DataType::second_type>::value>::type>
{
    using type = std::tuple<>;
};

template <typename DataType>
struct stack_tuple<
    DataType, typename std::enable_if<traits::treat_as_array_or_object_sink<DataType>::value>::type>
{
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<DataType>>(),
        std::declval<typename stack_tuple<typename DataType::value_type>::type>()));
};

template <typename ContainerType>
using container_stack_t = typename stack_tuple<ContainerType>::type;

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

template <typename VariantType, typename CharacterType> class token_handler
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

    virtual bool on_raw_number(const char* /*value*/, rapidjson::SizeType /*length*/)
    {
        return true;
    }

    virtual bool on_string(const char* /*value*/, rapidjson::SizeType /*length*/)
    {
        return true;
    }

    virtual bool on_object_start()
    {
        return true;
    }

    virtual bool on_key(const char* /*value*/, rapidjson::SizeType /*length*/)
    {
        return true;
    }

    virtual const std::basic_string<CharacterType>& get_key()
    {
        const static std::basic_string<CharacterType> dummy;
        return dummy;
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

template <typename ContainerType, typename VariantType, typename CharacterType>
class array_handler final : public token_handler<VariantType, CharacterType>
{
    using string_type = std::basic_string<CharacterType>;

  public:
    bool on_null() override
    {
        insert_null();
        return true;
    }

    bool on_bool(bool value) override
    {
        return insert_pod(value);
    }

    bool on_int(int value) override
    {
        return insert_pod(value);
    }

    bool on_uint(unsigned int value) override
    {
        return insert_pod(value);
    }

    bool on_int_64(std::int64_t value) override
    {
        return insert_pod(value);
    }

    bool on_uint_64(std::uint64_t value) override
    {
        return insert_pod(value);
    }

    bool on_double(double value) override
    {
        return insert_pod(value);
    }

    bool on_string(
        [[maybe_unused]] const CharacterType* value,
        [[maybe_unused]] rapidjson::SizeType length) override
    {
        if constexpr (traits::is_shared_ptr<typename ContainerType::value_type>::value) {
            using element_type = typename ContainerType::value_type::element_type;
            if constexpr (std::is_same_v<string_type, element_type>) {
                string_type data(value, length);
                insert(m_container, std::make_shared<string_type>(std::move(data)));
            }
        }

        if constexpr (traits::is_unique_ptr<typename ContainerType::value_type>::value) {
            using element_type = typename ContainerType::value_type::element_type;
            if constexpr (std::is_same_v<string_type, element_type>) {
                string_type data(value, length);
                insert(m_container, std::make_unique<string_type>(std::move(data)));
            }
        }

        if constexpr (std::is_same_v<string_type, typename ContainerType::value_type>) {
            string_type data(value, length);
            insert(m_container, std::move(data));
        }

        return true;
    }

    VariantType get_container() override
    {
        return { &m_container };
    }

  private:
    void insert_null()
    {
        if constexpr (
            traits::is_unique_ptr<typename ContainerType::value_type>::value ||
            traits::is_shared_ptr<typename ContainerType::value_type>::value) {
            insert(m_container, nullptr);
        }
    }

    template <typename DataType> bool insert_pod(DataType value)
    {
        // The various static casts in this function are necessary to keep warnings and static
        // assertions at bay.

        if constexpr (traits::is_shared_ptr<typename ContainerType::value_type>::value) {
            using element_type = typename ContainerType::value_type::element_type;
            if constexpr (std::is_convertible_v<decltype(value), element_type>) {
                insert(
                    m_container, std::make_shared<element_type>(static_cast<element_type>(value)));
            }
        }

        if constexpr (traits::is_unique_ptr<typename ContainerType::value_type>::value) {
            using element_type = typename ContainerType::value_type::element_type;
            if constexpr (std::is_convertible_v<decltype(value), element_type>) {
                insert(
                    m_container, std::make_unique<element_type>(static_cast<element_type>(value)));
            }
        }

        if constexpr (std::is_convertible_v<decltype(value), typename ContainerType::value_type>) {
            insert(m_container, static_cast<typename ContainerType::value_type>(value));
        }

        return true;
    }

    ContainerType m_container;
};

template <typename ContainerType, typename VariantType, typename CharacterType>
class object_handler final : public token_handler<VariantType, CharacterType>
{
    using string_type = std::basic_string<CharacterType>;

  public:
    bool on_null() override
    {
        construct_pair(nullptr);
        return true;
    }

    bool on_bool(bool value) override
    {
        construct_pair(value);
        return true;
    }

    bool on_int(int value) override
    {
        construct_pair(value);
        return true;
    }

    bool on_uint(unsigned int value) override
    {
        construct_pair(value);
        return true;
    }

    bool on_int_64(std::int64_t value) override
    {
        construct_pair(value);
        return true;
    }

    bool on_uint_64(std::uint64_t value) override
    {
        construct_pair(value);
        return true;
    }

    bool on_double(double value) override
    {
        construct_pair(value);
        return true;
    }

    bool on_string(
        [[maybe_unused]] const CharacterType* value,
        [[maybe_unused]] rapidjson::SizeType length) override
    {
        if constexpr (std::is_same_v<string_type, typename decltype(m_value)::element_type>) {
            finalize_pair_and_insert(string_type(value, length));
        }

        return true;
    }

    bool on_key(const CharacterType* value, rapidjson::SizeType length) override
    {
        m_key = string_type(value, length);
        return true;
    }

    VariantType get_container() override
    {
        return { &m_container };
    }

    const std::basic_string<CharacterType>& get_key() override
    {
        return m_key;
    }

  private:
    template <typename DataType> void construct_pair(DataType&& value)
    {
        // Splitting pair construction into two functions and using SFINAE to hide
        // inapplicable code-gen seems to work well to keep warnings at bay.

        if constexpr (std::is_convertible_v<
                          decltype(value), typename decltype(m_value)::element_type>) {
            using value_type = typename decltype(m_value)::element_type;
            finalize_pair_and_insert(static_cast<value_type>(value));
        }
    }

    template <typename DataType> void finalize_pair_and_insert(DataType&& value)
    {
        using value_type = typename decltype(m_value)::element_type;
        m_value = std::make_unique<value_type>(std::forward<DataType>(value));

        assert(!m_key.empty());
        insert(m_container, std::make_pair(std::move(m_key), std::move(*m_value)));
    }

    static_assert(traits::is_pair<typename ContainerType::value_type>::value, "");

    string_type m_key;
    std::unique_ptr<typename ContainerType::value_type::second_type> m_value;

    ContainerType m_container;
};

template <typename ContainerType, typename VariantType, typename CharacterType>
auto make_token_handler() -> typename std::enable_if<
    traits::treat_as_object_sink<ContainerType>::value,
    std::unique_ptr<token_handler<VariantType, CharacterType>>>::type
{
    return std::make_unique<object_handler<ContainerType, VariantType, CharacterType>>();
}

template <typename ContainerType, typename VariantType, typename CharacterType>
auto make_token_handler() -> typename std::enable_if<
    traits::treat_as_array_sink<ContainerType>::value,
    std::unique_ptr<token_handler<VariantType, CharacterType>>>::type
{
    return std::make_unique<array_handler<ContainerType, VariantType, CharacterType>>();
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

template <typename TupleType, typename VariantType, typename CharacterType> class factory
{
    constexpr static std::size_t tuple_size = std::tuple_size<TupleType>::value;

  public:
    std::unique_ptr<token_handler<VariantType, CharacterType>> operator()(int index)
    {
        return produce(index);
    }

  private:
    std::unique_ptr<token_handler<VariantType, CharacterType>> produce(int index)
    {
        return produce_impl(make_indices<tuple_size>(), index);
    }

    template <int I, int... Is>
    std::unique_ptr<token_handler<VariantType, CharacterType>>
    produce_impl(const indices<I, Is...>&, int index)
    {
        if (I == index) {
            using container_type = typename std::tuple_element<I, TupleType>::type;
            return make_token_handler<container_type, VariantType, CharacterType>();
        }

        return produce_impl(indices<Is...>(), index);
    }

    std::unique_ptr<token_handler<VariantType, CharacterType>>
    produce_impl(const indices<>&, int /*index*/)
    {
        throw std::runtime_error("Out of range");
    }
};

template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename ContainerType, typename EncodingType>
class delegating_handler final
    : public rapidjson::BaseReaderHandler<
          EncodingType, delegating_handler<ContainerType, typename EncodingType::Ch>>
{
    using character_type = typename EncodingType::Ch;

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

    bool
    RawNumber(const character_type* const value, rapidjson::SizeType length, bool /*should_copy*/)
    {
        return m_handlers[m_index]->on_raw_number(value, length);
    }

    bool String(const character_type* const value, rapidjson::SizeType length, bool /*should_copy*/)
    {
        return m_handlers[m_index]->on_string(value, length);
    }

    bool StartObject()
    {
        ++m_index;
        auto handler = m_handler_factory(m_index);
        m_handlers.emplace_back(std::move(handler));

        return true;
    }

    bool Key(const character_type* const value, rapidjson::SizeType length, bool /*should_copy*/)
    {
        return m_handlers[m_index]->on_key(value, length);
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
    template <typename SourceType, typename SinkType>
    void funnel_source_to_sink(SourceType* const source, SinkType* const sink)
    {
        assert(sink != nullptr);
        assert(source != nullptr);

        using sink_type = std::remove_pointer_t<decltype(sink)>;
        using source_type = std::remove_pointer_t<decltype(source)>;

        static_assert(traits::is_container<sink_type>::value, "Expected a container.");

        if constexpr (traits::is_pair<typename sink_type::value_type>::value) {
            using sink_value_type = typename sink_type::value_type::second_type;
            if constexpr (std::is_same_v<source_type, sink_value_type>) {
                const auto& key = m_handlers[m_handlers.size() - 2]->get_key();
                insert(*sink, std::make_pair(key, std::move(*source)));
            }
        }

        if constexpr (std::is_same_v<source_type, typename sink_type::value_type>) {
            insert(*sink, std::move(*source));
        }
    }

    template <typename SourceType, typename SinkType>
    void delegate_source(SourceType* const source, const SinkType& sink_variant)
    {
        std::visit(
            overloaded{ [](const std::monostate&) {},
                        [&](auto* const sink) { funnel_source_to_sink(source, sink); } },
            sink_variant);
    }

    bool finalize_container()
    {
        assert(!m_handlers.empty());

        if (m_index - 1 < 0) {
            auto variant = m_handlers.back()->get_container();
            m_container = std::move(*std::get<ContainerType*>(variant));
        } else {
            const auto source_variant = m_handlers[m_handlers.size() - 1]->get_container();
            const auto sink_variant = m_handlers[m_handlers.size() - 2]->get_container();

            std::visit(
                overloaded{ [](const std::monostate&) {},
                            [&](auto* const source) { delegate_source(source, sink_variant); } },
                source_variant);
        }

        m_handlers.pop_back();
        --m_index;

        return true;
    }

    using container_stack = container_stack_t<ContainerType>;
    using container_variant = container_variant_t<container_stack>;
    factory<container_stack, container_variant, character_type> m_handler_factory;

    std::vector<std::unique_ptr<token_handler<container_variant, character_type>>> m_handlers;

    std::int32_t m_index = -1;

    ContainerType m_container;
};

template <typename ContainerType> void parse_json(const char* const json, ContainerType& container)
{
    rapidjson::Reader reader;
    delegating_handler<ContainerType, rapidjson::UTF8<>> handler;
    rapidjson::StringStream stream{ json };

    if (reader.Parse(stream, handler)) {
        container = std::move(*handler.get_container());
    } else {
        const auto offset = std::to_string(reader.GetErrorOffset());
        const auto errorCode = reader.GetParseErrorCode();
        const auto parseError = std::string{ rapidjson::GetParseError_En(errorCode) };

        auto message = "Error: " + parseError + "at offset " + offset + ".";
        throw std::runtime_error{ std::move(message) };
    }
}
} // namespace detail

namespace sax_deserializer
{
template <typename ContainerType> auto from_json(const char* const json)
{
    ContainerType container;
    detail::parse_json(json, container);

    return container;
}
} // namespace sax_deserializer
} // namespace json_utils

#endif
