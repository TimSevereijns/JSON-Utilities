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

#include "future_std.h"
#include "json_traits.h"

namespace json_utils
{
namespace serializer
{
template <typename WriterType> void to_json(WriterType& writer, bool data);

template <typename WriterType> void to_json(WriterType& writer, std::int32_t data);

template <typename WriterType> void to_json(WriterType& writer, std::uint32_t data);

template <typename WriterType> void to_json(WriterType& writer, std::int64_t data);

template <typename WriterType> void to_json(WriterType& writer, std::uint64_t data);

template <typename WriterType, typename DataType>
auto to_json(WriterType& writer, DataType data) ->
    typename std::enable_if<std::is_floating_point<DataType>::value>::type;

template <typename WriterType, typename CharacterType, typename CharacterTraits, typename Allocator>
void to_json(
    WriterType& writer, const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

template <typename WriterType>
auto to_json(WriterType& writer, const char* const data) ->
    typename std::enable_if<std::is_same<char, typename WriterType::Ch>::value>::type;

template <typename WriterType>
auto to_json(WriterType& writer, const char* const data) ->
    typename std::enable_if<std::is_same<char16_t, typename WriterType::Ch>::value>::type;

template <typename WriterType>
auto to_json(WriterType& writer, const char* const data) ->
    typename std::enable_if<std::is_same<char32_t, typename WriterType::Ch>::value>::type;

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::shared_ptr<DataType>& pointer);

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::unique_ptr<DataType>& pointer);

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::weak_ptr<DataType>& pointer);

template <typename WriterType, typename CharacterType, typename CharacterTraits, typename Allocator>
void to_json(
    WriterType& writer, const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

template <typename WriterType, typename ContainerType>
auto to_json(WriterType& writer, const ContainerType& container) ->
    typename std::enable_if<traits::treat_as_array<ContainerType>::value>::type;

template <typename WriterType, typename ContainerType>
auto to_json(WriterType& writer, const ContainerType& container) ->
    typename std::enable_if<traits::treat_as_object<ContainerType>::value>::type;

template <typename WriterType, typename FirstType, typename SecondType>
void to_json(WriterType& writer, const std::pair<FirstType, SecondType>& pair);

#if __cplusplus >= 201703L // C++17
template <typename WriterType, typename CharacterType, typename CharacterTraits>
void to_json(
    WriterType& writer, const std::basic_string_view<CharacterType, CharacterTraits>& view);

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::optional<DataType>& data);

template <typename WriterType> void to_json(WriterType& writer, const std::filesystem::path& path);
#endif
} // namespace serializer

namespace deserializer
{
template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace_back<ContainerType>,
        traits::treat_as_array<ContainerType>>::value>::type;

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace<ContainerType>, traits::treat_as_array<ContainerType>>::value>::type;

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace_back<ContainerType>,
        traits::treat_as_object<ContainerType>>::value>::type;

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) ->
    typename std::enable_if<future_std::conjunction<
        traits::has_emplace<ContainerType>, traits::treat_as_object<ContainerType>>::value>::type;
} // namespace deserializer
} // namespace json_utils
