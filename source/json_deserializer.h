#pragma once

#include "json_fwd.h"

#include <stdexcept>

namespace json_utils
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
    } // namespace detail

    namespace deserializer
    {
        struct back_inserter_policy
        {
            template <typename DataType, typename ContainerType>
            static void insert(DataType&& data, ContainerType& container)
            {
                container.emplace_back(std::forward<DataType>(data));
            }
        };

        struct default_inserter_policy
        {
            template <typename DataType, typename ContainerType>
            static void insert(DataType&& data, ContainerType& container)
            {
                container.emplace(std::forward<DataType>(data));
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

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename ContainerType::second_type, int>::value,
                std::pair<std::string, int>>::type
        {
            return std::make_pair<std::string, int>(member.name.GetString(), member.value.GetInt());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::uint32_t>::value,
                std::pair<std::string, std::uint32_t>>::type
        {
            return std::make_pair<std::string, std::uint32_t>(
                member.name.GetString(), member.value.GetUint());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::int64_t>::value,
                std::pair<std::string, std::int64_t>>::type
        {
            return std::make_pair<std::string, std::int64_t>(
                member.name.GetString(), member.value.GetInt64());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::uint64_t>::value,
                std::pair<std::string, std::uint64_t>>::type
        {
            return std::make_pair<std::string, std::uint64_t>(
                member.name.GetString(), member.value.GetUint64());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, double>::value,
                std::pair<std::string, double>>::type
        {
            return std::make_pair<std::string, double>(
                member.name.GetString(), member.value.GetDouble());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::string>::value,
                std::pair<std::string, std::string>>::type
        {
            return std::make_pair<std::string, std::string>(
                member.name.GetString(), member.value.GetString());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, bool>::value,
                std::pair<std::string, bool>>::type
        {
            return std::make_pair<std::string, bool>(
                member.name.GetString(), member.value.GetBool());
        }

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        std::pair<std::string, ContainerType>
        construct_nested_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
        {
            static_assert(
                std::is_default_constructible<ContainerType>::value,
                "The container must have a default constructible.");

            ContainerType container;

            using deserializer::from_json;
            from_json(member.value, container);

            return std::make_pair<std::string, ContainerType>(
                member.name.GetString(), std::move(container));
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
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
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
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

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, bool>::value>::type
        {
            if (!value.IsBool()) {
                throw std::invalid_argument(
                    "Expected a boolean, got " + detail::type_to_string(value) + ".");
            }

            insert<InsertionPolicy>(value.GetBool(), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, int>::value>::type
        {
            if (!value.IsInt()) {
                throw std::invalid_argument(
                    "Expected an integer, got " + detail::type_to_string(value) + ".");
            }

            insert<InsertionPolicy>(value.GetInt(), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, unsigned int>::value>::type
        {
            if (!value.IsUint()) {
                throw std::invalid_argument(
                    "Expected an unsigned integer, got " + detail::type_to_string(value) + ".");
            }

            insert<InsertionPolicy>(value.GetUint(), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, std::int64_t>::value>::type
        {
            if (!value.IsInt64()) {
                throw std::invalid_argument(
                    "Expected a 64-bit integer, got " + detail::type_to_string(value) + ".");
            }

            insert<InsertionPolicy>(value.GetInt64(), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, std::uint64_t>::value>::type
        {
            if (!value.IsUint64()) {
                throw std::invalid_argument(
                    "Expected an unsigned, 64-bit integer, got " + detail::type_to_string(value) +
                    ".");
            }

            insert<InsertionPolicy>(value.GetUint64(), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, std::string>::value>::type
        {
            if (!value.IsString()) {
                throw std::invalid_argument(
                    "Expected a string, got " + detail::type_to_string(value) + ".");
            }

            insert<InsertionPolicy>(value.GetString(), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, double>::value>::type
        {
            if (!value.IsDouble()) {
                throw std::invalid_argument(
                    "Expected a double, got " + detail::type_to_string(value) + ".");
            }

            insert<InsertionPolicy>(value.GetDouble(), container);
        }

        template <
            typename InsertionPolicy, typename EncodingType, typename AllocatorType,
            typename ContainerType>
        void dispatch_insertion(
            const rapidjson::GenericMember<EncodingType, AllocatorType>& member,
            ContainerType& container)
        {
            auto pair = to_key_value_pair<typename ContainerType::value_type>(member);
            insert<InsertionPolicy>(std::move(pair), container);
        }

        template <
            typename InsertionPolicy, typename ContainerType, typename EncodingType,
            typename AllocatorType>
        auto dispatch_insertion(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<
                traits::treat_as_array<typename ContainerType::value_type>::value>::type
        {
            using nested_container_type = typename ContainerType::value_type;

            static_assert(
                std::is_default_constructible<nested_container_type>::value,
                "The container must have a default constructible.");

            nested_container_type nested_container;

            using deserializer::from_json;
            from_json(json_value, nested_container);

            insert<InsertionPolicy>(std::move(nested_container), container);
        }

        template <
            typename InsertionPolicy, typename ContainerType, typename EncodingType,
            typename AllocatorType>
        auto dispatch_insertion(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<
                traits::treat_as_object<typename ContainerType::value_type>::value>::type
        {
            using nested_container_type = typename ContainerType::value_type;

            static_assert(
                std::is_default_constructible<nested_container_type>::value,
                "The container must have a default constructible.");

            nested_container_type nested_container;

            using deserializer::from_json;
            from_json(json_value, nested_container);

            insert<InsertionPolicy>(std::move(nested_container), container);
        }

        template <
            typename InsertionPolicy, typename ContainerType, typename EncodingType,
            typename AllocatorType>
        auto deserialize_json_object(
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
            typename InsertionPolicy, typename ContainerType, typename EncodingType,
            typename AllocatorType>
        auto deserialize_json_array(
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

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_array<ContainerType>>::value>::type
        {
            deserialize_json_array<back_inserter_policy>(json_value, container);
        }

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace<ContainerType>,
                traits::treat_as_array<ContainerType>>::value>::type
        {
            deserialize_json_array<default_inserter_policy>(json_value, container);
        }

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_object<ContainerType>>::value>::type
        {
            deserialize_json_object<back_inserter_policy>(json_value, container);
        }

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace<ContainerType>,
                traits::treat_as_object<ContainerType>>::value>::type
        {
            deserialize_json_object<default_inserter_policy>(json_value, container);
        }
    } // namespace deserializer
} // namespace json_utils
