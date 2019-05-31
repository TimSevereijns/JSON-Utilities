#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>

#include "json_traits.h"

namespace json_utils
{
    namespace serializer
    {
        template <typename Writer> void to_json(Writer& writer, bool data);

        template <typename Writer> void to_json(Writer& writer, std::int32_t data);

        template <typename Writer> void to_json(Writer& writer, std::uint32_t data);

        template <typename Writer> void to_json(Writer& writer, std::int64_t data);

        template <typename Writer> void to_json(Writer& writer, std::uint64_t data);

        template <typename Writer, typename DataType>
        auto to_json(Writer& writer, DataType data) ->
            typename std::enable_if<std::is_floating_point<DataType>::value>::type;

        template <
            typename Writer, typename CharacterType, typename CharacterTraits, typename Allocator>
        void to_json(
            Writer& writer,
            const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char, typename Writer::Ch>::value>::type;

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char16_t, typename Writer::Ch>::value>::type;

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char32_t, typename Writer::Ch>::value>::type;

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::shared_ptr<Type>& pointer);

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::unique_ptr<Type>& pointer);

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::weak_ptr<Type>& pointer);

        template <
            typename Writer, typename CharacterType, typename CharacterTraits, typename Allocator>
        void to_json(
            Writer& writer,
            const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

        template <typename Writer, typename Type>
        auto to_json(Writer& writer, const Type& container) ->
            typename std::enable_if<traits::treat_as_array<Type>::value>::type;

        template <typename Writer, typename Type>
        auto to_json(Writer& writer, const Type& container) ->
            typename std::enable_if<traits::treat_as_object<Type>::value>::type;

        template <typename Writer, typename FirstType, typename SecondType>
        void to_json(Writer& writer, const std::pair<FirstType, SecondType>& pair);

#if __cplusplus >= 201703L // C++17
        template <typename Writer, typename CharacterType, typename CharacterTraits>
        void
        to_json(Writer& writer, const std::basic_string_view<CharacterType, CharacterTraits>& view);

        template <typename Writer, typename DataType>
        void to_json(Writer& writer, const std::optional<DataType>& data);

        template <typename Writer> void to_json(Writer& writer, const std::filesystem::path& path);
#endif
    } // namespace serializer

    namespace deserializer
    {
        template <
            typename InsertionPolicy, typename EncodingType, typename AllocatorType,
            typename ContainerType>
        void dispatch_insertion(
            const rapidjson::GenericMember<EncodingType, AllocatorType>& member,
            ContainerType& container);

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_array<ContainerType>>::value>::type;

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace<ContainerType>,
                traits::treat_as_array<ContainerType>>::value>::type;

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_object<ContainerType>>::value>::type;

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto from_json(
            const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
            ContainerType& container) ->
            typename std::enable_if<std::conjunction<
                traits::has_emplace<ContainerType>,
                traits::treat_as_object<ContainerType>>::value>::type;
    } // namespace deserializer
} // namespace json_utils
