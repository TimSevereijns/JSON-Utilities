#pragma once

#include "json_fwd.h"

#include <stdexcept>

namespace json_utils
{
namespace deserializer
{
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
} // namespace deserializer

namespace deserializer
{
namespace detail
{
std::string type_to_string(const rapidjson::Value& value)
{
    switch (value.GetType()) {
        case rapidjson::Type::kArrayType:
            return "an array";
        case rapidjson::Type::kFalseType:
            return "a false type";
        case rapidjson::Type::kNullType:
            return "a null type";
        case rapidjson::Type::kNumberType:
            return "a numeric type";
        case rapidjson::Type::kObjectType:
            return "an object type";
        case rapidjson::Type::kStringType:
            return "a string type";
        case rapidjson::Type::kTrueType:
            return "a true type";
    }

    return "an unknown type";
}

template <typename DataType> struct value_extractor
{
    template <typename EncodingType, typename AllocatorType>
    static bool extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        throw std::invalid_argument("Cannot extract unsupported type");
    }
};

template <> struct value_extractor<bool>
{
    template <typename EncodingType, typename AllocatorType>
    static bool extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (!value.IsBool()) {
            throw std::invalid_argument(
                "Expected a bool, got " + detail::type_to_string(value) + ".");
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
        if (!value.IsInt()) {
            throw std::invalid_argument(
                "Expected a 32-bit integer, got " + detail::type_to_string(member.value) + ".");
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
        if (!value.IsUint()) {
            throw std::invalid_argument(
                "Expected an unsigned, 32-bit integer, got " +
                detail::type_to_string(member.value) + ".");
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
        if (!value.IsInt64()) {
            throw std::invalid_argument(
                "Expected a 64-bit integer, got " + detail::type_to_string(member.value) + ".");
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
        if (!value.IsUint64()) {
            throw std::invalid_argument(
                "Expected an unsigned, 64-bit integer, got " +
                detail::type_to_string(member.value) + ".");
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
        if (!value.IsDouble()) {
            throw std::invalid_argument(
                "Expected a real, got " + detail::type_to_string(member.value) + ".");
        }

        return value.GetDouble();
    }
};

template <> struct value_extractor<std::string>
{
    template <typename EncodingType, typename AllocatorType>
    static std::string
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (!value.IsString()) {
            throw std::invalid_argument(
                "Expected a string, got " + detail::type_to_string(member.value) + ".");
        }

        return value.GetString();
    }
};

template <typename InsertionPolicy, typename DataType, typename ContainerType>
void insert(DataType&& value, ContainerType& container)
{
    static_assert(
        std::is_convertible<DataType, typename ContainerType::value_type>::value,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    InsertionPolicy::insert(std::forward<DataType>(value), container);
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member) ->
    typename std::enable_if<
        traits::treat_as_value<typename PairType::second_type>::value,
        std::pair<std::string, typename PairType::second_type>>::type
{
    return std::make_pair<std::string, typename PairType::second_type>(
        member.name.GetString(),
        value_extractor<typename PairType::second_type>::extract_or_throw(member.value));
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
std::pair<std::string, ContainerType>
construct_nested_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
{
    static_assert(
        std::is_default_constructible<ContainerType>::value,
        "Nested container must be default constructible.");

    ContainerType container;

    using deserializer::from_json;
    from_json(member.value, container);

    return std::make_pair<std::string, ContainerType>(
        member.name.GetString(), std::move(container));
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member) ->
    typename std::enable_if<
        traits::treat_as_object<typename PairType::second_type>::value,
        std::pair<std::string, typename PairType::second_type>>::type
{
    if (!member.value.IsObject()) {
        throw std::invalid_argument(
            "Expected an object, got " + detail::type_to_string(member.value) + ".");
    }

    using nested_container_type = typename PairType::second_type;
    return construct_nested_pair<nested_container_type>(member);
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member) ->
    typename std::enable_if<
        traits::treat_as_array<typename PairType::second_type>::value,
        std::pair<std::string, typename PairType::second_type>>::type
{
    if (!member.value.IsArray()) {
        throw std::invalid_argument(
            "Expected an array, got " + detail::type_to_string(member.value) + ".");
    }

    using nested_container_type = typename PairType::second_type;
    return construct_nested_pair<nested_container_type>(member);
}

template <
    typename InsertionPolicy, typename ContainerType, typename EncodingType, typename AllocatorType>
auto dispatch_insertion(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& value, ContainerType& container) ->
    typename std::enable_if<traits::treat_as_value<typename ContainerType::value_type>::value>::type
{
    using desired_type = typename ContainerType::value_type;
    insert<InsertionPolicy>(value_extractor<desired_type>::extract_or_throw(value), container);
}

template <
    typename InsertionPolicy, typename EncodingType, typename AllocatorType, typename ContainerType>
void dispatch_insertion(
    const rapidjson::GenericMember<EncodingType, AllocatorType>& member, ContainerType& container)
{
    auto pair = to_key_value_pair<typename ContainerType::value_type>(member);
    insert<InsertionPolicy>(std::move(pair), container);
}

template <
    typename InsertionPolicy, typename ContainerType, typename EncodingType, typename AllocatorType>
auto dispatch_insertion(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::disjunction<
        traits::treat_as_array<typename ContainerType::value_type>,
        traits::treat_as_object<typename ContainerType::value_type>>::value>::type
{
    static_assert(
        std::is_default_constructible<typename ContainerType::value_type>::value,
        "Nested container must be default constructible.");

    using nested_container_type = typename ContainerType::value_type;
    nested_container_type nested_container;

    using deserializer::from_json;
    from_json(json_value, nested_container);

    insert<InsertionPolicy>(std::move(nested_container), container);
}

template <
    typename InsertionPolicy, typename ContainerType, typename EncodingType, typename AllocatorType>
void deserialize_json_object(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
{
    if (!json_value.IsObject()) {
        throw std::invalid_argument(
            "Expected an object, got " + detail::type_to_string(json_value) + ".");
    }

    const auto& json_object = json_value.GetObject();
    for (const auto& json_value : json_object) {
        dispatch_insertion<InsertionPolicy>(json_value, container);
    }
}

template <
    typename InsertionPolicy, typename ContainerType, typename EncodingType, typename AllocatorType>
void deserialize_json_array(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
{
    if (!json_value.IsArray()) {
        throw std::invalid_argument(
            "Expected an array, got " + detail::type_to_string(json_value) + ".");
    }

    const auto& json_array = json_value.GetArray();
    for (const auto& json_value : json_array) {
        dispatch_insertion<InsertionPolicy>(json_value, container);
    }
}
} // namespace detail

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace_back<ContainerType>,
        traits::treat_as_array<ContainerType>>::value>::type
{
    detail::deserialize_json_array<back_insertion_policy>(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace<ContainerType>, traits::treat_as_array<ContainerType>>::value>::type
{
    detail::deserialize_json_array<default_insertion_policy>(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace_back<ContainerType>,
        traits::treat_as_object<ContainerType>>::value>::type
{
    detail::deserialize_json_object<back_insertion_policy>(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace<ContainerType>, traits::treat_as_object<ContainerType>>::value>::type
{
    detail::deserialize_json_object<default_insertion_policy>(json_value, container);
}
} // namespace deserializer
} // namespace json_utils
