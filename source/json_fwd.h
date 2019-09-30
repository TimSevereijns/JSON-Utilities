#pragma once

#if __cplusplus >= 201703L // C++17
#include <filesystem>
#endif

#include <string>
#include <type_traits>
#include <utility>

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
template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer, bool data);

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::int32_t data);

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::uint32_t data);

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::int64_t data);

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::uint64_t data);

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    DataType data) -> typename std::enable_if<std::is_floating_point<DataType>::value>::type;

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename CharacterType, typename CharacterTraits, typename Allocator>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const char* const data) ->
    typename std::enable_if<std::is_same<
        char, typename rapidjson::Writer<
                  OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value>::type;

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const char* const data) ->
    typename std::enable_if<std::is_same<
        char16_t, typename rapidjson::Writer<
                      OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value>::type;

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const char* const data) ->
    typename std::enable_if<std::is_same<
        char32_t, typename rapidjson::Writer<
                      OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value>::type;

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::shared_ptr<DataType>& pointer);

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::unique_ptr<DataType>& pointer);

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::weak_ptr<DataType>& weakPointer);

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename ContainerType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const ContainerType& container) ->
    typename std::enable_if<traits::treat_as_array<ContainerType>::value>::type;

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename ContainerType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const ContainerType& container) ->
    typename std::enable_if<traits::treat_as_object<ContainerType>::value>::type;

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename FirstType, typename SecondType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::pair<FirstType, SecondType>& pair);

#if __cplusplus >= 201703L // C++17

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename CharacterType, typename CharacterTraits>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::basic_string_view<CharacterType, CharacterTraits>& view);

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::optional<DataType>& data);

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::filesystem::path& path);

#endif
} // namespace serializer

namespace deserializer
{
namespace detail
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
} // namespace detail
} // namespace deserializer
} // namespace json_utils
