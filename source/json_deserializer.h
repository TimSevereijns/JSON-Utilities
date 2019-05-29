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

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                traits::treat_as_object<typename PairType::second_type>::value,
                std::pair<std::string, typename PairType::second_type>>::type
        {
            assert(member.value.IsObject());

            using nested_container_type = typename PairType::second_type;

            auto container =
                from_json_object<inserter_policy, nested_container_type>(member.value.GetObject());

            return std::make_pair<std::string, nested_container_type>(
                member.name.GetString(), std::move(container));
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                traits::treat_as_array<typename PairType::second_type>::value,
                std::pair<std::string, typename PairType::second_type>>::type
        {
            assert(member.value.IsArray());

            using nested_container_type = typename PairType::second_type;

            auto container = from_json_array<back_inserter_policy, nested_container_type>(
                member.value.GetArray());

            return std::make_pair<std::string, nested_container_type>(
                member.name.GetString(), std::move(container));
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, bool>::value>::type
        {
            if (value.IsBool()) {
                insert<InsertionPolicy>(value.GetBool(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<typename ContainerType::value_type, int>::value>::type
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
                std::is_same<typename ContainerType::value_type, unsigned int>::value>::type
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
                std::is_same<typename ContainerType::value_type, std::int64_t>::value>::type
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
                std::is_same<typename ContainerType::value_type, std::uint64_t>::value>::type
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
                std::is_same<typename ContainerType::value_type, std::string>::value>::type
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
                std::is_same<typename ContainerType::value_type, double>::value>::type
        {
            if (value.IsDouble()) {
                insert<InsertionPolicy>(value.GetDouble(), container);
            } else {
                assert(false);
            }
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

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                traits::treat_as_array<typename ContainerType::value_type>::value>::type
        {
            using nested_container_type = typename ContainerType::value_type;

            if (value.IsArray()) {
                auto nested_container =
                    from_json_array<InsertionPolicy, nested_container_type>(value.GetArray());
                insert<InsertionPolicy>(std::move(nested_container), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType, typename DataType>
        ContainerType from_json_array(const rapidjson::GenericArray<true, DataType>& json_array)
        {
            ContainerType container;

            for (const auto& json_value : json_array) {
                dispatch_insertion<InsertionPolicy>(json_value, container);
            }

            return container;
        }

        template <typename InsertionPolicy, typename ContainerType, typename DataType>
        ContainerType from_json_object(const rapidjson::GenericObject<true, DataType>& json_object)
        {
            ContainerType container;

            for (const auto& json_member : json_object) {
                dispatch_insertion<InsertionPolicy>(json_member, container);
            }

            return container;
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document, ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_array<ContainerType>>::value>::type
        {
            if (!document.IsArray()) {
                return;
            }

            const auto& json_array = document.GetArray();
            for (const auto& json_value : json_array) {
                dispatch_insertion<back_inserter_policy>(json_value, container);
            }
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document, ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace<ContainerType>,
                traits::treat_as_array<ContainerType>>::value>::type
        {
            if (!document.IsArray()) {
                return;
            }

            const auto& json_array = document.GetArray();
            for (const auto& json_value : json_array) {
                dispatch_insertion<inserter_policy>(json_value, container);
            }
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document, ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_object<ContainerType>>::value>::type
        {
            if (!document.IsObject()) {
                return;
            }

            const auto& json_object = document.GetObject();
            for (const auto& json_value : json_object) {
                dispatch_insertion<back_inserter_policy>(json_value, container);
            }
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document, ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace<ContainerType>,
                traits::treat_as_object<ContainerType>>::value>::type
        {
            if (!document.IsObject()) {
                return;
            }

            const auto& json_object = document.GetObject();
            for (const auto& json_value : json_object) {
                dispatch_insertion<inserter_policy>(json_value, container);
            }
        }
    } // namespace deserializer
} // namespace json_utils
