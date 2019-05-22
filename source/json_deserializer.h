#pragma once

#include "json_fwd.h"

namespace json_utils
{
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

        struct inserter_policy
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
                std::disjunction<
                    std::is_same<DataType, typename ContainerType::value_type>,
                    std::is_convertible<DataType, typename ContainerType::value_type>>::value,
                "The type being inserted is not the same as, or cannot be converted to, the "
                "container's value type.");

            InsertionPolicy::insert(std::forward<DataType>(value), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<int, typename ContainerType::value_type>::value>::type
        {
            if (value.IsInt()) {
                insert<InsertionPolicy>(value.GetInt(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<unsigned int, typename ContainerType::value_type>::value>::type
        {
            if (value.IsUint()) {
                insert<InsertionPolicy>(value.GetUint(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<std::int64_t, typename ContainerType::value_type>::value>::type
        {
            if (value.IsInt64()) {
                insert<InsertionPolicy>(value.GetInt64(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<std::uint64_t, typename ContainerType::value_type>::value>::type
        {
            if (value.IsUint64()) {
                insert<InsertionPolicy>(value.GetUint64(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<std::string, typename ContainerType::value_type>::value>::type
        {
            if (value.IsString()) {
                insert<InsertionPolicy>(value.GetString(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<double, typename ContainerType::value_type>::value>::type
        {
            if (value.IsDouble()) {
                insert<InsertionPolicy>(value.GetDouble(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType, typename DataType>
        auto from_json_array(const rapidjson::GenericArray<true, DataType>& jsonArray)
        {
            ContainerType container{};

            for (const auto& jsonValue : jsonArray) {
                dispatch_insertion<InsertionPolicy>(jsonValue, container);
            }

            return container;
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                traits::treat_as_array<typename ContainerType::value_type>::value>::type
        {
            using nested_container_type = typename ContainerType::value_type;

            if (value.IsArray()) {
                auto nestedContainer =
                    from_json_array<InsertionPolicy, nested_container_type>(value.GetArray());
                insert<InsertionPolicy>(std::move(nestedContainer), container);
            } else {
                assert(false);
            }
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

        template <
            typename InsertionPolicy, typename EncodingType, typename AllocatorType,
            typename ContainerType>
        auto dispatch_insertion(
            const rapidjson::GenericMember<EncodingType, AllocatorType>& member,
            ContainerType& container)
        {
            auto pair = to_key_value_pair<typename ContainerType::value_type>(member);
            insert<InsertionPolicy>(std::move(pair), container);
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_array<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container;

            if (document.IsArray()) {
                for (const auto& jsonValue : document.GetArray()) {
                    dispatch_insertion<back_inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace<ContainerType>, traits::treat_as_array<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container;

            if (document.IsArray()) {
                for (const auto& jsonValue : document.GetArray()) {
                    dispatch_insertion<inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_object<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container;

            if (document.IsObject()) {
                for (const auto& jsonValue : document.GetObject()) {
                    dispatch_insertion<back_inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace<ContainerType>, traits::treat_as_object<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container;

            if (document.IsObject()) {
                for (const auto& jsonValue : document.GetObject()) {
                    dispatch_insertion<inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }
    } // namespace deserializer
} // namespace json_utils
