#pragma once

#if __cplusplus >= 201703L // C++17
#include <filesystem>
#include <optional>
#endif

#include <memory>
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
namespace detail
{
template <typename WriterType> void to_json(WriterType& writer, bool data);

template <typename WriterType> void to_json(WriterType& writer, std::int32_t data);

template <typename WriterType> void to_json(WriterType& writer, std::uint32_t data);

template <typename WriterType> void to_json(WriterType& writer, std::int64_t data);

template <typename WriterType> void to_json(WriterType& writer, std::uint64_t data);

template <typename WriterType, typename DataType>
auto to_json(WriterType& writer, DataType data)
    -> std::enable_if_t<std::is_floating_point<DataType>::value>;

template <typename WriterType, typename CharacterType, typename CharacterTraits, typename Allocator>
void to_json(
    WriterType& writer, const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

template <typename WriterType> void to_json(WriterType& writer, const char* data);

template <typename WriterType> void to_json(WriterType& writer, const wchar_t* data);

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::shared_ptr<DataType>& pointer);

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::unique_ptr<DataType>& pointer);

template <typename WriterType, typename DataType>
void to_json(WriterType& writer, const std::weak_ptr<DataType>& weakPointer);

template <typename WriterType, typename ContainerType>
auto to_json(WriterType& writer, const ContainerType& container)
    -> std::enable_if_t<traits::treat_as_array_sink_v<ContainerType>>;

template <typename WriterType, typename ContainerType>
auto to_json(WriterType& writer, const ContainerType& container)
    -> std::enable_if_t<traits::treat_as_object_sink_v<ContainerType>>;

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
} // namespace detail
} // namespace serializer

namespace dom_deserializer
{
namespace detail
{
template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) -> std::enable_if_t<traits::treat_as_array_sink_v<ContainerType>>;

template <typename ContainerType, typename EncodingType, typename AllocatorType>
auto from_json(
    const rapidjson::GenericValue<EncodingType, AllocatorType>& json_value,
    ContainerType& container) -> std::enable_if_t<traits::treat_as_object_sink_v<ContainerType>>;
} // namespace detail
} // namespace dom_deserializer
} // namespace json_utils
