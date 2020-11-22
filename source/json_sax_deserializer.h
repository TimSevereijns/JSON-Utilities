#pragma once

#if __cplusplus >= 201703L // C++17

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>

#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "json_traits.h"

namespace json_utils
{
namespace sax_deserializer
{
namespace detail
{
template <typename DataType, typename = void> struct container_stack;

template <typename DataType>
struct container_stack<
    DataType, typename std::enable_if<
                  !traits::treat_as_array_or_object_sink<DataType>::value &&
                  !traits::is_pair<DataType>::value>::type>
{
    using type = std::tuple<>;
};

template <typename DataType>
struct container_stack<
    DataType, typename std::enable_if<
                  traits::is_pair<DataType>::value &&
                  traits::is_container<typename DataType::second_type>::value>::type>
{
    using type = typename container_stack<typename DataType::second_type>::type;
};

template <typename DataType>
struct container_stack<
    DataType, typename std::enable_if<
                  traits::is_pair<DataType>::value &&
                  !traits::is_container<typename DataType::second_type>::value>::type>
{
    using type = std::tuple<>;
};

template <typename DataType>
struct container_stack<
    DataType, typename std::enable_if<traits::treat_as_array_or_object_sink<DataType>::value>::type>
{
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<DataType>>(),
        std::declval<typename container_stack<typename DataType::value_type>::type>()));
};

template <typename ContainerType>
using container_stack_t = typename container_stack<ContainerType>::type;

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
    virtual ~token_handler() = default;

    virtual void on_default()
    {
    }

    virtual void on_null()
    {
    }

    virtual void on_bool(bool /*value*/)
    {
    }

    virtual void on_int(int /*value*/)
    {
    }

    virtual void on_uint(unsigned int /*value*/)
    {
    }

    virtual void on_int_64(std::int64_t /*value*/)
    {
    }

    virtual void on_uint_64(std::uint64_t /*value*/)
    {
    }

    virtual void on_double(double /*value*/)
    {
    }

    virtual void on_raw_number(const CharacterType* const /*value*/, rapidjson::SizeType /*length*/)
    {
    }

    virtual void on_string(const CharacterType* const /*value*/, rapidjson::SizeType /*length*/)
    {
    }

    virtual void on_object_start()
    {
    }

    virtual void on_key(const CharacterType* const /*value*/, rapidjson::SizeType /*length*/)
    {
    }

    virtual const std::basic_string<CharacterType>& get_key()
    {
        const static std::basic_string<CharacterType> dummy;
        return dummy;
    }

    virtual void on_object_end(rapidjson::SizeType /*length*/)
    {
    }

    virtual void on_array_start()
    {
    }

    virtual void on_array_end(rapidjson::SizeType /*length*/)
    {
    }

    virtual VariantType get_container()
    {
        return { std::monostate{} };
    }
};

template <typename ContainerType, typename VariantType, typename CharacterType>
class array_handler final : public token_handler<VariantType, CharacterType>
{
    static_assert(!traits::is_pair<typename ContainerType::value_type>::value);

    using string_type = std::basic_string<CharacterType>;

  public:
    void on_null() override
    {
        if constexpr (
            traits::is_unique_ptr<typename ContainerType::value_type>::value ||
            traits::is_shared_ptr<typename ContainerType::value_type>::value) {
            insert(m_container, nullptr);
        }

        if constexpr (traits::is_optional<typename ContainerType::value_type>::value) {
            insert(m_container, std::nullopt);
        }
    }

    void on_bool(bool value) override
    {
        insert_pod(value);
    }

    void on_int(int value) override
    {
        insert_pod(value);
    }

    void on_uint(unsigned int value) override
    {
        insert_pod(value);
    }

    void on_int_64(std::int64_t value) override
    {
        insert_pod(value);
    }

    void on_uint_64(std::uint64_t value) override
    {
        insert_pod(value);
    }

    void on_double(double value) override
    {
        insert_pod(value);
    }

    void on_raw_number(const CharacterType* const value, rapidjson::SizeType length) override
    {
        on_string(value, length);
    }

    void on_string(
        [[maybe_unused]] const CharacterType* const value,
        [[maybe_unused]] rapidjson::SizeType length) override
    {
        using target_type = typename ContainerType::value_type;

        if constexpr (traits::is_shared_ptr<target_type>::value) {
            using element_type = typename target_type::element_type;
            if constexpr (std::is_same_v<string_type, element_type>) {
                insert(m_container, std::make_shared<string_type>(value, length));
            }
        } else if constexpr (traits::is_unique_ptr<target_type>::value) {
            using element_type = typename target_type::element_type;
            if constexpr (std::is_same_v<string_type, element_type>) {
                insert(m_container, std::make_unique<string_type>(value, length));
            }
        } else if constexpr (traits::is_optional<target_type>::value) {
            using element_type = typename target_type::value_type;
            if constexpr (std::is_convertible_v<decltype(value), element_type>) {
                insert(m_container, std::optional<string_type>(std::in_place, value, length));
            }
        } else if constexpr (std::is_same_v<string_type, target_type>) {
            insert(m_container, string_type{ value, length });
        }
    }

    VariantType get_container() override
    {
        return { &m_container };
    }

  private:
    template <typename DataType> void insert_pod([[maybe_unused]] DataType value)
    {
        using target_type = typename ContainerType::value_type;

        if constexpr (traits::is_shared_ptr<target_type>::value) {
            using element_type = typename target_type::element_type;
            if constexpr (std::is_convertible_v<DataType, element_type>) {
                insert(
                    m_container, std::make_shared<element_type>(static_cast<element_type>(value)));
            }
        } else if constexpr (traits::is_unique_ptr<target_type>::value) {
            using element_type = typename target_type::element_type;
            if constexpr (std::is_convertible_v<DataType, element_type>) {
                insert(
                    m_container, std::make_unique<element_type>(static_cast<element_type>(value)));
            }
        } else if constexpr (traits::is_optional<target_type>::value) {
            using element_type = typename target_type::value_type;
            if constexpr (std::is_convertible_v<DataType, element_type>) {
                insert(m_container, std::optional<element_type>(static_cast<element_type>(value)));
            }
        } else if constexpr (std::is_convertible_v<DataType, target_type>) {
            insert(m_container, static_cast<target_type>(value));
        }
    }

    ContainerType m_container;
};

template <typename ContainerType, typename VariantType, typename CharacterType>
class object_handler final : public token_handler<VariantType, CharacterType>
{
    static_assert(traits::is_pair<typename ContainerType::value_type>::value);

    using string_type = std::basic_string<CharacterType>;

  public:
    void on_null() override
    {
        if constexpr (
            traits::is_unique_ptr<typename decltype(m_value)::element_type>::value ||
            traits::is_shared_ptr<typename decltype(m_value)::element_type>::value) {
            finalize_pair_and_insert(nullptr);
        }

        if constexpr (traits::is_optional<typename decltype(m_value)::element_type>::value) {
            finalize_pair_and_insert(std::nullopt);
        }
    }

    void on_bool(bool value) override
    {
        construct_pair(value);
    }

    void on_int(int value) override
    {
        construct_pair(value);
    }

    void on_uint(unsigned int value) override
    {
        construct_pair(value);
    }

    void on_int_64(std::int64_t value) override
    {
        construct_pair(value);
    }

    void on_uint_64(std::uint64_t value) override
    {
        construct_pair(value);
    }

    void on_double(double value) override
    {
        construct_pair(value);
    }

    void on_raw_number(const CharacterType* const value, rapidjson::SizeType length) override
    {
        on_string(value, length);
    }

    void on_string(
        [[maybe_unused]] const CharacterType* const value,
        [[maybe_unused]] rapidjson::SizeType length) override
    {
        using element_type = typename ContainerType::value_type::second_type;

        if constexpr (traits::is_shared_ptr<element_type>::value) {
            using target_type = typename ContainerType::value_type::second_type::element_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                finalize_pair_and_insert(std::make_shared<target_type>(value, length));
            }
        } else if constexpr (traits::is_unique_ptr<element_type>::value) {
            using target_type = typename ContainerType::value_type::second_type::element_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                finalize_pair_and_insert(std::make_unique<target_type>(value, length));
            }
        } else if constexpr (traits::is_optional<element_type>::value) {
            using target_type = typename ContainerType::value_type::second_type::value_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                finalize_pair_and_insert(std::optional<target_type>(std::in_place, value, length));
            }
        } else if constexpr (std::is_same_v<string_type, element_type>) {
            finalize_pair_and_insert(string_type(value, length));
        }
    }

    void on_key(const CharacterType* const value, rapidjson::SizeType length) override
    {
        m_key = string_type(value, length);
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
    template <typename DataType> void construct_pair([[maybe_unused]] DataType value)
    {
        using element_type = typename ContainerType::value_type::second_type;

        if constexpr (traits::is_shared_ptr<element_type>::value) {
            using target_type = typename ContainerType::value_type::second_type::element_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                finalize_pair_and_insert(
                    std::make_shared<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (traits::is_unique_ptr<element_type>::value) {
            using target_type = typename ContainerType::value_type::second_type::element_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                finalize_pair_and_insert(
                    std::make_unique<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (traits::is_optional<element_type>::value) {
            using target_type = typename ContainerType::value_type::second_type::value_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                finalize_pair_and_insert(static_cast<target_type>(value));
            }
        } else if constexpr (std::is_convertible_v<DataType, element_type>) {
            using target_type = typename decltype(m_value)::element_type;
            finalize_pair_and_insert(static_cast<target_type>(value));
        }
    }

    template <typename DataType> void finalize_pair_and_insert(DataType&& value)
    {
        using target_type = typename decltype(m_value)::element_type;
        m_value = std::make_unique<target_type>(std::forward<DataType>(value));

        assert(!m_key.empty());
        insert(m_container, std::make_pair(std::move(m_key), std::move(*m_value)));
    }

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
    std::unique_ptr<token_handler<VariantType, CharacterType>> produce(int index)
    {
        return produce_impl(make_indices<tuple_size>(), index);
    }

  private:
    template <int I, int... Is>
    std::unique_ptr<token_handler<VariantType, CharacterType>>
    produce_impl(const indices<I, Is...>&, int index)
    {
        if (I == index) {
            using container_type = typename std::tuple_element<I, TupleType>::type;
            return make_token_handler<container_type, VariantType, CharacterType>();
        }

        return produce_impl(indices<Is...>{}, index);
    }

    std::unique_ptr<token_handler<VariantType, CharacterType>>
    produce_impl(const indices<>&, int /*index*/)
    {
        throw std::runtime_error{ "Out of range" };
    }
};

template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename ContainerType, typename EncodingType>
class delegating_handler final : public rapidjson::BaseReaderHandler<
                                     EncodingType, delegating_handler<ContainerType, EncodingType>>
{
    using character_type = typename EncodingType::Ch;

  public:
    bool Default()
    {
        return true;
    }

    bool Null()
    {
        validate_state();
        m_handlers[m_index]->on_null();
        return true;
    }

    bool Bool(bool value)
    {
        validate_state();
        m_handlers[m_index]->on_bool(value);
        return true;
    }

    bool Int(int value)
    {
        validate_state();
        m_handlers[m_index]->on_int(value);
        return true;
    }

    bool Uint(unsigned int value)
    {
        validate_state();
        m_handlers[m_index]->on_uint(value);
        return true;
    }

    bool Int64(std::int64_t value)
    {
        validate_state();
        m_handlers[m_index]->on_int_64(value);
        return true;
    }

    bool Uint64(std::uint64_t value)
    {
        validate_state();
        m_handlers[m_index]->on_uint_64(value);
        return true;
    }

    bool Double(double value)
    {
        validate_state();
        m_handlers[m_index]->on_double(value);
        return true;
    }

    bool
    RawNumber(const character_type* const value, rapidjson::SizeType length, bool /*should_copy*/)
    {
        validate_state();
        m_handlers[m_index]->on_raw_number(value, length);
        return true;
    }

    bool String(const character_type* const value, rapidjson::SizeType length, bool /*should_copy*/)
    {
        validate_state();
        m_handlers[m_index]->on_string(value, length);
        return true;
    }

    bool StartObject()
    {
        ++m_index;
        auto handler = m_handler_factory.produce(m_index);
        m_handlers.emplace_back(std::move(handler));

        return true;
    }

    bool Key(const character_type* const value, rapidjson::SizeType length, bool /*should_copy*/)
    {
        validate_state();
        m_handlers[m_index]->on_key(value, length);
        return true;
    }

    bool EndObject(rapidjson::SizeType /*length*/)
    {
        validate_state();
        finalize_container();
        return true;
    }

    bool StartArray()
    {
        ++m_index;
        auto handler = m_handler_factory.produce(m_index);
        m_handlers.emplace_back(std::move(handler));

        return true;
    }

    bool EndArray(rapidjson::SizeType /*length*/)
    {
        validate_state();
        finalize_container();
        return true;
    }

    ContainerType* get_container()
    {
        return &m_container;
    }

  private:
    RAPIDJSON_FORCEINLINE void validate_state()
    {
        if (RAPIDJSON_UNLIKELY(m_index < 0)) {
            throw std::runtime_error("Unexpected token.");
        }
    }

    template <typename SourceType, typename SinkType>
    void funnel_source_to_sink(SourceType* const source, SinkType* const sink)
    {
        assert(sink != nullptr);
        assert(source != nullptr);

        using sink_type = std::remove_pointer_t<decltype(sink)>;
        using source_type = std::remove_pointer_t<decltype(source)>;

        static_assert(
            traits::is_container<sink_type>::value, "The sink does not appear to be a container.");

        if constexpr (traits::is_pair<typename sink_type::value_type>::value) {
            using sink_value_type = typename sink_type::value_type::second_type;
            if constexpr (std::is_same_v<source_type, sink_value_type>) {
                const auto& key = m_handlers[m_handlers.size() - 2]->get_key();
                insert(*sink, std::make_pair(key, std::move(*source)));
            }
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

template <typename EncodingType, unsigned int ParsingFlags, typename ContainerType>
void parse_json(const typename EncodingType::Ch* const json, ContainerType& container)
{
    rapidjson::GenericReader<EncodingType, EncodingType> reader;
    delegating_handler<ContainerType, EncodingType> handler;
    rapidjson::GenericStringStream<EncodingType> stream{ json };

    if (reader.template Parse<ParsingFlags>(stream, handler)) {
        container = std::move(*handler.get_container());
    } else {
        const auto errorCode = reader.GetParseErrorCode();
        const auto parseError = std::string{ rapidjson::GetParseError_En(errorCode) };
        const auto offset = std::to_string(reader.GetErrorOffset());
        throw std::runtime_error{ "Error: " + parseError + "at offset " + offset + "." };
    }
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
auto from_json(const char* const json)
{
    ContainerType container;
    parse_json<rapidjson::UTF8<>, ParseFlags>(json, container);

    return container;
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
auto from_json(const wchar_t* const json)
{
    ContainerType container;
    parse_json<rapidjson::UTF16<>, ParseFlags>(json, container);

    return container;
}
} // namespace detail
} // namespace sax_deserializer
} // namespace json_utils

#endif
