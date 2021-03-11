#pragma once

#if __cplusplus >= 201703L // C++17

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>

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
template <typename DataType, typename = void> struct peeled_container;

template <typename DataType>
struct peeled_container<
    DataType,
    std::enable_if_t<
        !traits::treat_as_array_or_object_sink_v<DataType> && !traits::is_pair_v<DataType>>>
{
    using type = std::tuple<>;
};

template <typename DataType>
struct peeled_container<
    DataType,
    std::enable_if_t<
        traits::is_pair_v<DataType> && traits::is_container_v<typename DataType::second_type>>>
{
    using type = typename peeled_container<typename DataType::second_type>::type;
};

template <typename DataType>
struct peeled_container<
    DataType,
    std::enable_if_t<
        traits::is_pair_v<DataType> && !traits::is_container_v<typename DataType::second_type>>>
{
    using type = std::tuple<>;
};

template <typename DataType>
struct peeled_container<
    DataType, std::enable_if_t<traits::treat_as_array_or_object_sink_v<DataType>>>
{
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<DataType>>(),
        std::declval<typename peeled_container<typename DataType::value_type>::type>()));
};

template <typename ContainerType>
using peeled_container_t = typename peeled_container<ContainerType>::type;

template <typename Tuple> struct container_variant;

template <typename... Ts> struct container_variant<std::tuple<Ts...>>
{
    using type = std::variant<std::monostate, typename std::add_pointer<Ts>::type...>;
};

template <typename TupleType>
using container_variant_t = typename container_variant<TupleType>::type;

template <typename ContainerType, typename ElementType>
void insert(ContainerType& container, ElementType&& element)
{
    static_assert(
        std::is_convertible_v<std::decay_t<ElementType>, typename ContainerType::value_type>,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    if constexpr (traits::has_emplace_v<ContainerType>) {
        container.emplace(std::forward<ElementType>(element));
    } else if constexpr (traits::has_emplace_back_v<ContainerType>) {
        container.emplace_back(std::forward<ElementType>(element));
    }
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
    static_assert(!traits::is_pair_v<typename ContainerType::value_type>);

    using string_type = std::basic_string<CharacterType>;

  public:
    void on_null() override
    {
        using sink_type = typename ContainerType::value_type;

        if constexpr (traits::is_unique_ptr_v<sink_type> || traits::is_shared_ptr_v<sink_type>) {
            insert(m_container, nullptr);
        } else if constexpr (traits::is_optional_v<sink_type>) {
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
        using sink_type = typename ContainerType::value_type;

        if constexpr (traits::is_shared_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_same_v<string_type, target_type>) {
                insert(m_container, std::make_shared<string_type>(value, length));
            }
        } else if constexpr (traits::is_unique_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_same_v<string_type, target_type>) {
                insert(m_container, std::make_unique<string_type>(value, length));
            }
        } else if constexpr (traits::is_optional_v<sink_type>) {
            using target_type = typename sink_type::value_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                insert(m_container, std::optional<string_type>(std::in_place, value, length));
            }
        } else if constexpr (std::is_same_v<string_type, sink_type>) {
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
        using sink_type = typename ContainerType::value_type;

        if constexpr (traits::is_shared_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                insert(m_container, std::make_shared<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (traits::is_unique_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                insert(m_container, std::make_unique<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (traits::is_optional_v<sink_type>) {
            using target_type = typename sink_type::value_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                insert(m_container, std::optional<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (std::is_convertible_v<DataType, sink_type>) {
            insert(m_container, static_cast<sink_type>(value));
        }
    }

    ContainerType m_container;
};

template <typename ContainerType, typename VariantType, typename CharacterType>
class object_handler final : public token_handler<VariantType, CharacterType>
{
    static_assert(traits::is_pair_v<typename ContainerType::value_type>);

    using string_type = std::basic_string<CharacterType>;

  public:
    void on_null() override
    {
        using sink_type = typename ContainerType::value_type::second_type;

        if constexpr (traits::is_unique_ptr_v<sink_type> || traits::is_shared_ptr_v<sink_type>) {
            finalize_pair_and_insert(nullptr);
        } else if constexpr (traits::is_optional_v<sink_type>) {
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
        using sink_type = typename ContainerType::value_type::second_type;

        if constexpr (traits::is_shared_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                finalize_pair_and_insert(std::make_shared<target_type>(value, length));
            }
        } else if constexpr (traits::is_unique_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                finalize_pair_and_insert(std::make_unique<target_type>(value, length));
            }
        } else if constexpr (traits::is_optional_v<sink_type>) {
            using target_type = typename sink_type::value_type;
            if constexpr (std::is_convertible_v<decltype(value), target_type>) {
                finalize_pair_and_insert(std::optional<target_type>(std::in_place, value, length));
            }
        } else if constexpr (std::is_same_v<string_type, sink_type>) {
            finalize_pair_and_insert(string_type{ value, length });
        }
    }

    void on_key(const CharacterType* const value, rapidjson::SizeType length) override
    {
        m_key = string_type{ value, length };
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
        using sink_type = typename ContainerType::value_type::second_type;

        if constexpr (traits::is_shared_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                finalize_pair_and_insert(
                    std::make_shared<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (traits::is_unique_ptr_v<sink_type>) {
            using target_type = typename sink_type::element_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                finalize_pair_and_insert(
                    std::make_unique<target_type>(static_cast<target_type>(value)));
            }
        } else if constexpr (traits::is_optional_v<sink_type>) {
            using target_type = typename sink_type::value_type;
            if constexpr (std::is_convertible_v<DataType, target_type>) {
                finalize_pair_and_insert(static_cast<target_type>(value));
            }
        } else if constexpr (std::is_convertible_v<DataType, sink_type>) {
            finalize_pair_and_insert(static_cast<sink_type>(value));
        }
    }

    template <typename DataType> void finalize_pair_and_insert(DataType&& value)
    {
        using sink_type = typename ContainerType::value_type::second_type;
        m_value = std::make_unique<sink_type>(std::forward<DataType>(value));

        assert(!m_key.empty());
        insert(m_container, std::make_pair(std::move(m_key), std::move(*m_value)));
    }

    string_type m_key;
    std::unique_ptr<typename ContainerType::value_type::second_type> m_value;

    ContainerType m_container;
};

template <typename ContainerType, typename VariantType, typename CharacterType>
std::unique_ptr<token_handler<VariantType, CharacterType>> make_token_handler()
{
    if constexpr (traits::treat_as_object_sink_v<ContainerType>) {
        return std::make_unique<object_handler<ContainerType, VariantType, CharacterType>>();
    }

    if constexpr (traits::treat_as_array_sink_v<ContainerType>) {
        return std::make_unique<array_handler<ContainerType, VariantType, CharacterType>>();
    }
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
            using container_type = std::tuple_element_t<I, TupleType>;
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

template <typename... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
            throw std::runtime_error{ "Unexpected token." };
        }
    }

    template <typename SourceType, typename SinkType>
    void funnel_source_to_sink(SourceType* const source, SinkType* const sink)
    {
        assert(sink != nullptr);
        assert(source != nullptr);

        static_assert(
            traits::is_container<SinkType>::value, "The sink does not appear to be a container.");

        if constexpr (traits::is_pair_v<typename SinkType::value_type>) {
            using sink_value_type = typename SinkType::value_type::second_type;
            if constexpr (std::is_same_v<SourceType, sink_value_type>) {
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

    void finalize_container()
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
    }

    using peeled_container = peeled_container_t<ContainerType>;
    using container_variant = container_variant_t<peeled_container>;
    factory<peeled_container, container_variant, character_type> m_handler_factory;

    std::vector<std::unique_ptr<token_handler<container_variant, character_type>>> m_handlers;

    std::int32_t m_index = -1;

    ContainerType m_container;
};

template <typename EncodingType, unsigned int ParsingFlags, typename StreamType, typename ContainerType>
void parse_stream(StreamType& stream, ContainerType& container)
{
    rapidjson::GenericReader<EncodingType, EncodingType> reader;
    delegating_handler<ContainerType, EncodingType> handler;

    if (reader.template Parse<ParsingFlags>(stream, handler)) {
        container = std::move(*handler.get_container());
    } else {
        const auto errorCode = reader.GetParseErrorCode();
        const auto parseError = std::string{ rapidjson::GetParseError_En(errorCode) };
        const auto offset = std::to_string(reader.GetErrorOffset());
        throw std::runtime_error{ "Error: " + parseError + " at offset " + offset + "." };
    }
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
ContainerType from_json(const char* const json)
{
    rapidjson::GenericStringStream<rapidjson::UTF8<>> stream{ json };

    ContainerType container;
    parse_stream<rapidjson::UTF8<>, ParseFlags>(stream, container);

    return container;
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
ContainerType from_json(const wchar_t* const json)
{
    rapidjson::GenericStringStream<rapidjson::UTF16<>> stream{ json };

    ContainerType container;
    parse_stream<rapidjson::UTF16<>, ParseFlags>(stream, container);

    return container;
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
ContainerType from_json(const std::filesystem::path& path)
{
    std::ifstream file_stream{ path };
    rapidjson::IStreamWrapper stream_wrapper{ file_stream };

    ContainerType container;
    parse_stream<rapidjson::UTF8<>, ParseFlags>(stream_wrapper, container);

    return container;
}
} // namespace detail
} // namespace sax_deserializer
} // namespace json_utils

#endif
