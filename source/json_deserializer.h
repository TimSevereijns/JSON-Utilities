#pragma once

#include "json_fwd.h"

#include <stdexcept>

namespace json_utils
{
namespace deserializer
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
} // namespace detail

// Template variables are required to have external linkage per the Standard.
template <typename DataType> constexpr DataType make_odr_safe{};

namespace
{
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
constexpr const auto& from_json = make_odr_safe<detail::from_json_functor>;
}

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
inline std::string type_to_string(const rapidjson::Value& value)
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
    JSON_UTILS_NORETURN static bool
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& /*value*/)
    {
        throw std::invalid_argument("Cannot extract unsupported type");
    }
};

template <> struct value_extractor<bool>
{
    template <typename EncodingType, typename AllocatorType>
    JSON_UTILS_NODISCARD static bool
    extract_or_throw(const rapidjson::GenericValue<EncodingType, AllocatorType>& value)
    {
        if (!value.IsBool()) {
            throw std::invalid_argument("Expected a bool, got " + type_to_string(value) + ".");
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
                "Expected a 32-bit integer, got " + type_to_string(value) + ".");
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
                "Expected an unsigned, 32-bit integer, got " + type_to_string(value) + ".");
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
                "Expected a 64-bit integer, got " + type_to_string(value) + ".");
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
                "Expected an unsigned, 64-bit integer, got " + type_to_string(value) + ".");
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
            throw std::invalid_argument("Expected a real, got " + type_to_string(value) + ".");
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
            throw std::invalid_argument("Expected a string, got " + type_to_string(value) + ".");
        }

        return value.GetString();
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

template <typename InsertionPolicy, typename DataType, typename ContainerType>
void insert(DataType&& value, ContainerType& container)
{
    static_assert(
        std::is_convertible<DataType, typename ContainerType::value_type>::value,
        "The type being inserted is not the same as, or cannot be converted to, the "
        "container's value type.");

    InsertionPolicy::insert(std::forward<DataType>(value), container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
std::pair<std::string, ContainerType>
construct_nested_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
{
    static_assert(
        std::is_default_constructible<ContainerType>::value,
        "Nested container must be default constructible.");

    ContainerType container;
    deserializer::from_json(member.value, container);

    return std::make_pair<std::string, ContainerType>(
        member.name.GetString(), std::move(container));
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member) ->
    typename std::enable_if<
        traits::treat_as_value<typename PairType::second_type>::value,
        std::pair<std::string, typename PairType::second_type>>::type
{
    using desired_type = typename PairType::second_type;

    return std::make_pair<std::string, desired_type>(
        member.name.GetString(), value_extractor<desired_type>::extract_or_throw(member.value));
}

template <typename PairType, typename EncodingType, typename AllocatorType>
auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member) ->
    typename std::enable_if<
        traits::treat_as_object<typename PairType::second_type>::value,
        std::pair<std::string, typename PairType::second_type>>::type
{
    if (!member.value.IsObject()) {
        throw std::invalid_argument(
            "Expected an object, got " + type_to_string(member.value) + ".");
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
        throw std::invalid_argument("Expected an array, got " + type_to_string(member.value) + ".");
    }

    using nested_container_type = typename PairType::second_type;
    return construct_nested_pair<nested_container_type>(member);
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
    const rapidjson::GenericValue<EncodingType, AllocatorType>& value, ContainerType& container) ->
    typename std::enable_if<traits::treat_as_value<typename ContainerType::value_type>::value>::type
{
    using desired_type = typename ContainerType::value_type;
    insert<InsertionPolicy>(value_extractor<desired_type>::extract_or_throw(value), container);
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
    deserializer::from_json(json_value, nested_container);

    insert<InsertionPolicy>(std::move(nested_container), container);
}

template <
    typename InsertionPolicy, typename ContainerType, typename EncodingType, typename AllocatorType>
void deserialize_json_object(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
{
    if (!json_value.IsObject()) {
        throw std::invalid_argument("Expected an object, got " + type_to_string(json_value) + ".");
    }

    const auto& json_object = json_value.GetObject();
    for (const auto& nested_json_value : json_object) {
        dispatch_insertion<InsertionPolicy>(nested_json_value, container);
    }
}

template <
    typename InsertionPolicy, typename ContainerType, typename EncodingType, typename AllocatorType>
void deserialize_json_array(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container)
{
    if (!json_value.IsArray()) {
        throw std::invalid_argument("Expected an array, got " + type_to_string(json_value) + ".");
    }

    const auto& json_array = json_value.GetArray();
    for (const auto& nested_json_value : json_array) {
        dispatch_insertion<InsertionPolicy>(nested_json_value, container);
    }
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace_back<ContainerType>,
        traits::treat_as_array<ContainerType>>::value>::type
{
    deserialize_json_array<back_insertion_policy>(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace<ContainerType>, traits::treat_as_array<ContainerType>>::value>::type
{
    deserialize_json_array<default_insertion_policy>(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace_back<ContainerType>,
        traits::treat_as_object<ContainerType>>::value>::type
{
    deserialize_json_object<back_insertion_policy>(json_value, container);
}

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace<ContainerType>, traits::treat_as_object<ContainerType>>::value>::type
{
    deserialize_json_object<default_insertion_policy>(json_value, container);
}
} // namespace detail
} // namespace deserializer
} // namespace json_utils
