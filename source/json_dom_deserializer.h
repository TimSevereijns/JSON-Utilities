#pragma once

#include "json_fwd.h"

#include <stdexcept>

namespace json_utils
{
namespace dom_deserializer
{
namespace detail
{
struct from_json_functor
{
    template <typename ContainerType, typename EncodingType, typename AllocatorType>
    void operator()(
        const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
        ContainerType& container) const
    {
        from_json(json_value, container);
    }

    template <typename ContainerType, typename EncodingType, typename AllocatorType>
    void operator()(
        const rapidjson::GenericMember<EncodingType, AllocatorType>& json_value,
        ContainerType& container) const
    {
        from_json(json_value, container);
    }
};

// Template variables are required to have external linkage per the Standard.
template <typename DataType> constexpr DataType make_odr_safe{};
} // namespace detail

namespace
{
// Using a Niebloid customization point.
//
// Variables declared at global scope will have external linkage, so we'll need to use an anonymous
// namespace to keep the enclosed reference "itself from being multiply defined." This works
// because anonymous namespaces behave as if a unique identifier were chosen for each translation
// unit in which it appears. As a result, the reference has internal linkage. However, since the
// reference below refers to a variable template (which is required to have external linkage), "all
// translation units [will] refer to the same entity," and therefore "there is no ODR violation."
//
// Source: Suggested Design for Customization Points
// [http://ericniebler.github.io/std/wg21/D4381.html]
//
// @note Use an `inline constexpr` variable when upgrading to C++17.
constexpr const auto& from_json = detail::make_odr_safe<detail::from_json_functor>;
} // namespace

struct default_insertion_policy
{
    template <typename DataType, typename ContainerType>
    static void insert(DataType&& data, ContainerType& container)
    {
        container.emplace(std::forward<DataType>(data));
    }
};

struct back_insertion_policy
{
    template <typename DataType, typename ContainerType>
    static void insert(DataType&& data, ContainerType& container)
    {
        container.emplace_back(std::forward<DataType>(data));
    }
};

namespace detail
{
template <typename JsonValueType> std::string type_to_string(const JsonValueType& value)
{
    switch (value.GetType()) {
        case rapidjson::Type::kArrayType:
            return "an array";
        case rapidjson::Type::kTrueType:
        case rapidjson::Type::kFalseType:
            return "a boolean";
        case rapidjson::Type::kNullType:
            return "null";
        case rapidjson::Type::kNumberType:
            return "a numeric type";
        case rapidjson::Type::kObjectType:
            return "an object";
        case rapidjson::Type::kStringType:
            return "a string";
    }

    return "an unknown type";
}

template <
    typename StringType, typename InputEncodingType, typename OutputEncodingType,
    typename EncodingType, typename AllocatorType>
StringType transcode(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
{
    assert(value.IsString());

    rapidjson::GenericStringStream<InputEncodingType> source{ value.GetString() };
    rapidjson::GenericStringBuffer<OutputEncodingType> target;

    using transcoder_type = rapidjson::Transcoder<InputEncodingType, OutputEncodingType>;

    bool successfully_transcoded = true;
    while (source.Peek() != '\0' && successfully_transcoded) {
        successfully_transcoded = transcoder_type::Transcode(source, target);
    }

    if (RAPIDJSON_UNLIKELY(!successfully_transcoded)) {
        throw std::invalid_argument{ "Failed to transcode strings." };
    }

    return target.GetString();
}

template <typename DataType> struct value_extractor
{
    template <typename EncodingType, typename AllocatorType>
    static bool
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& /*value*/)
    {
        throw std::invalid_argument{ "Cannot extract unsupported type" };
    }
};

template <> struct value_extractor<bool>
{
    template <typename EncodingType, typename AllocatorType>
    static bool extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (RAPIDJSON_UNLIKELY(!value.IsBool())) {
            throw std::invalid_argument{ "Expected a bool, got " + type_to_string(value) + "." };
        }

        return value.GetBool();
    }
};

template <> struct value_extractor<std::int32_t>
{
    template <typename EncodingType, typename AllocatorType>
    static std::int32_t
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (RAPIDJSON_UNLIKELY(!value.IsInt())) {
            throw std::invalid_argument{ "Expected a 32-bit integer, got " + type_to_string(value) +
                                         "." };
        }

        return value.GetInt();
    }
};

template <> struct value_extractor<std::uint32_t>
{
    template <typename EncodingType, typename AllocatorType>
    static std::uint32_t
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (RAPIDJSON_UNLIKELY(!value.IsUint())) {
            throw std::invalid_argument{ "Expected an unsigned, 32-bit integer, got " +
                                         type_to_string(value) + "." };
        }

        return value.GetUint();
    }
};

template <> struct value_extractor<std::int64_t>
{
    template <typename EncodingType, typename AllocatorType>
    static std::int64_t
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (RAPIDJSON_UNLIKELY(!value.IsInt64())) {
            throw std::invalid_argument{ "Expected a 64-bit integer, got " + type_to_string(value) +
                                         "." };
        }

        return value.GetInt64();
    }
};

template <> struct value_extractor<std::uint64_t>
{
    template <typename EncodingType, typename AllocatorType>
    static std::uint64_t
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (RAPIDJSON_UNLIKELY(!value.IsUint64())) {
            throw std::invalid_argument{ "Expected an unsigned, 64-bit integer, got " +
                                         type_to_string(value) + "." };
        }

        return value.GetUint64();
    }
};

template <> struct value_extractor<double>
{
    template <typename EncodingType, typename AllocatorType>
    static double
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (RAPIDJSON_UNLIKELY(!value.IsDouble())) {
            throw std::invalid_argument{ "Expected a real, got " + type_to_string(value) + "." };
        }

        return value.GetDouble();
    }
};

template <> struct value_extractor<std::string>
{
    using value_type = std::string;

    template <typename EncodingType, typename AllocatorType>
    static auto extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
        -> std::enable_if_t<std::is_same<typename EncodingType::Ch, char>::value, value_type>
    {
        if (RAPIDJSON_UNLIKELY(!value.IsString())) {
            throw std::invalid_argument{ "Expected a string, got " + type_to_string(value) + "." };
        }

        return value.GetString();
    }

    template <typename EncodingType, typename AllocatorType>
    static auto extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
        -> std::enable_if_t<std::is_same<typename EncodingType::Ch, wchar_t>::value, value_type>
    {
        if (RAPIDJSON_UNLIKELY(!value.IsString())) {
            throw std::invalid_argument{ "Expected a string, got " + type_to_string(value) + "." };
        }

        return transcode<value_type, rapidjson::UTF16<>, rapidjson::UTF8<>>(value);
    }
};

template <> struct value_extractor<std::wstring>
{
    using value_type = std::wstring;

    template <typename EncodingType, typename AllocatorType>
    static auto extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
        -> std::enable_if_t<std::is_same<typename EncodingType::Ch, wchar_t>::value, value_type>
    {
        if (RAPIDJSON_UNLIKELY(!value.IsString())) {
            throw std::invalid_argument{ "Expected a string, got " + type_to_string(value) + "." };
        }

        return value.GetString();
    }

    template <typename EncodingType, typename AllocatorType>
    static auto extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
        -> std::enable_if_t<std::is_same<typename EncodingType::Ch, char>::value, value_type>
    {
        if (RAPIDJSON_UNLIKELY(!value.IsString())) {
            throw std::invalid_argument{ "Expected a string, got " + type_to_string(value) + "." };
        }

        return transcode<value_type, rapidjson::UTF8<>, rapidjson::UTF16<>>(value);
    }
};

template <typename DataType> struct value_extractor<std::unique_ptr<DataType>>
{
    template <typename EncodingType, typename AllocatorType>
    static std::unique_ptr<DataType>
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (value.IsNull()) {
            return nullptr;
        }

        return std::make_unique<DataType>(value_extractor<DataType>::extract_or_throw(value));
    }
};

template <typename DataType> struct value_extractor<std::shared_ptr<DataType>>
{
    template <typename EncodingType, typename AllocatorType>
    static std::shared_ptr<DataType>
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (value.IsNull()) {
            return nullptr;
        }

        return std::make_shared<DataType>(value_extractor<DataType>::extract_or_throw(value));
    }
};

#if __cplusplus >= 201703L

template <typename DataType> struct value_extractor<std::optional<DataType>>
{
    template <typename EncodingType, typename AllocatorType>
    static std::optional<DataType>
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (value.IsNull()) {
            return std::nullopt;
        }

        return value_extractor<DataType>::extract_or_throw(value);
    }
};

#endif

template <typename DataType, typename ContainerType>
auto insert(DataType&& value, ContainerType& container)
    -> std::enable_if_t<traits::has_emplace_v<ContainerType>>
{
    static_assert(
        std::is_convertible<DataType, typename ContainerType::value_type>::value,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    container.emplace(std::forward<DataType>(value));
}

template <typename DataType, typename ContainerType>
auto insert(DataType&& value, ContainerType& container)
    -> std::enable_if_t<traits::has_emplace_back_v<ContainerType>>
{
    static_assert(
        std::is_convertible<DataType, typename ContainerType::value_type>::value,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    container.emplace_back(std::forward<DataType>(value));
}

template <typename PairType, typename EncodingType, typename AllocatorType>
PairType construct_nested_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
{
    using key_type = typename std::decay<typename PairType::first_type>::type;
    using nested_type = typename PairType::second_type;

    static_assert(
        std::is_default_constructible<nested_type>::value,
        "Nested container must be default constructible.");

    nested_type container;
    dom_deserializer::from_json(member.value, container);

    return { value_extractor<key_type>::extract_or_throw(member.name), std::move(container) };
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
    -> std::enable_if_t<traits::treat_as_value_sink_v<typename PairType::second_type>, PairType>
{
    using key_type = typename std::decay<typename PairType::first_type>::type;
    using value_type = typename PairType::second_type;

    return { value_extractor<key_type>::extract_or_throw(member.name),
             value_extractor<value_type>::extract_or_throw(member.value) };
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
    -> std::enable_if_t<traits::treat_as_object_sink_v<typename PairType::second_type>, PairType>
{
    if (!member.value.IsObject()) {
        throw std::invalid_argument{ "Expected an object, got " + type_to_string(member.value) +
                                     "." };
    }

    return construct_nested_pair<PairType>(member);
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
    -> std::enable_if_t<traits::treat_as_array_sink_v<typename PairType::second_type>, PairType>
{
    if (RAPIDJSON_UNLIKELY(!member.value.IsArray())) {
        throw std::invalid_argument{ "Expected an array, got " + type_to_string(member.value) +
                                     "." };
    }

    return construct_nested_pair<PairType>(member);
}

template <typename EncodingType, typename AllocatorType, typename ContainerType>
void dispatch_insertion(
    const rapidjson::GenericMember<EncodingType, AllocatorType>& member, ContainerType& container)
{
    auto pair = to_key_value_pair<typename ContainerType::value_type>(member);
    insert(std::move(pair), container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto dispatch_insertion(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& value, ContainerType& container)
    -> std::enable_if_t<traits::treat_as_value_sink_v<typename ContainerType::value_type>>
{
    using desired_type = typename ContainerType::value_type;
    insert(value_extractor<desired_type>::extract_or_throw(value), container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto dispatch_insertion(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
    -> std::enable_if_t<
        traits::treat_as_array_sink_v<typename ContainerType::value_type> ||
        traits::treat_as_object_sink_v<typename ContainerType::value_type>>
{
    static_assert(
        std::is_default_constructible<typename ContainerType::value_type>::value,
        "Nested container must be default constructible.");

    using nested_container_type = typename ContainerType::value_type;

    nested_container_type nested_container;
    dom_deserializer::from_json(json_value, nested_container);

    insert(std::move(nested_container), container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
void deserialize_json_object(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
{
    if (RAPIDJSON_UNLIKELY(!json_value.IsObject())) {
        throw std::invalid_argument{ "Expected an object, got " + type_to_string(json_value) +
                                     "." };
    }

    const auto& json_object = json_value.GetObject();
    for (const auto& nested_json_value : json_object) {
        dispatch_insertion(nested_json_value, container);
    }
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
void deserialize_json_array(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
{
    if (RAPIDJSON_UNLIKELY(!json_value.IsArray())) {
        throw std::invalid_argument{ "Expected an array, got " + type_to_string(json_value) + "." };
    }

    const auto& json_array = json_value.GetArray();
    for (const auto& nested_json_value : json_array) {
        dispatch_insertion(nested_json_value, container);
    }
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) -> std::enable_if_t<traits::treat_as_array_sink_v<ContainerType>>
{
    deserialize_json_array(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) -> std::enable_if_t<traits::treat_as_object_sink_v<ContainerType>>
{
    deserialize_json_object(json_value, container);
}
} // namespace detail
} // namespace dom_deserializer
} // namespace json_utils
