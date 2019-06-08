#pragma once

#include "json_fwd.h"

namespace json_utils
{
namespace serializer
{
inline std::string to_narrow_json_key(const std::string& data) noexcept
{
    return data;
}

inline std::wstring to_wide_json_key(const std::wstring& data) noexcept
{
    return data;
}

template <typename CharacterType> struct key_master
{
};

template <> struct key_master<char>
{
    template <typename DataType> static std::string generate_key(const DataType& data)
    {
        using serializer::to_narrow_json_key;
        return to_narrow_json_key(data);
    }
};

template <> struct key_master<wchar_t>
{
    template <typename DataType> static std::wstring generate_key(const DataType& data)
    {
        using serializer::to_wide_json_key;
        return to_wide_json_key(data);
    }
};
} // namespace serializer

namespace detail
{
template <typename Writer, typename KeyType, typename ValueType>
void insert_key_value_pair(Writer& writer, const KeyType& key, const ValueType& value)
{
    writer.Key(serializer::key_master<typename Writer::Ch>::generate_key(key).c_str());

    using serializer::to_json;
    to_json(writer, value);
}
} // namespace detail

namespace serializer
{
template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer, bool data)
{
    writer.Bool(data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::int32_t data)
{
    writer.Int(data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::uint32_t data)
{
    writer.Uint(data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::int64_t data)
{
    writer.Int64(data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    std::uint64_t data)
{
    writer.Uint64(data);
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    DataType data) -> typename std::enable_if<std::is_floating_point<DataType>::value>::type
{
    writer.Double(data);
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename CharacterType, typename CharacterTraits, typename Allocator>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::basic_string<CharacterType, CharacterTraits, Allocator>& data)
{
    static_assert(
        std::is_same<
            CharacterType,
            typename rapidjson::Writer<
                OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value,
        "The character type to be serialized differs from the character type of the "
        "rapidjson::Writer object.");

    writer.String(data.c_str());
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const char* const data) ->
    typename std::enable_if<std::is_same<
        char, typename rapidjson::Writer<
                  OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value>::type
{
    if (data == nullptr) {
        writer.Null();
        return;
    }

    writer.String(data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const char* const data) ->
    typename std::enable_if<std::is_same<
        char16_t, typename rapidjson::Writer<
                      OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value>::type
{
    if (data == nullptr) {
        writer.Null();
        return;
    }

    writer.String(data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const char* const data) ->
    typename std::enable_if<std::is_same<
        char32_t, typename rapidjson::Writer<
                      OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch>::value>::type
{
    if (data == nullptr) {
        writer.Null();
        return;
    }

    writer.String(data);
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::shared_ptr<DataType>& pointer)
{
    if (pointer == nullptr) {
        writer.Null();
        return;
    }

    using serializer::to_json;
    to_json(writer, *pointer);
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::unique_ptr<DataType>& pointer)
{
    if (pointer == nullptr) {
        writer.Null();
        return;
    }

    using serializer::to_json;
    to_json(writer, *pointer);
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::weak_ptr<DataType>& weakPointer)
{
    const auto strongPointer = weakPointer.lock();
    if (strongPointer == nullptr) {
        writer.Null();
        return;
    }

    using serializer::to_json;
    to_json(writer, *strongPointer);
}

// @todo Add overload for a vector of pairs

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename ContainerType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const ContainerType& container) ->
    typename std::enable_if<traits::treat_as_array<ContainerType>::value>::type
{
    writer.StartArray();

    for (const auto& item : container) {
        using serializer::to_json;
        to_json(writer, item);
    }

    writer.EndArray();
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename ContainerType>
auto to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const ContainerType& container) ->
    typename std::enable_if<traits::treat_as_object<ContainerType>::value>::type
{
    writer.StartObject();

    for (const auto& item : container) {
        using serializer::to_json;
        to_json(writer, item);
    }

    writer.EndObject();
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename FirstType, typename SecondType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::pair<FirstType, SecondType>& pair)
{
    detail::insert_key_value_pair(writer, pair.first, pair.second);
}

#if __cplusplus >= 201703L // C++17
template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename CharacterType, typename CharacterTraits>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::basic_string_view<CharacterType, CharacterTraits>& view)
{
    writer.String(view.data());
}

template <
    typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType,
    typename DataType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::optional<DataType>& data)
{
    if (!data.has_value()) {
        writer.Null();
        return;
    }

    using serializer::to_json;
    to_json(writer, *data);
}

template <typename OutputStreamType, typename SourceEncodingType, typename TargetEncodingType>
void to_json(
    rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>& writer,
    const std::filesystem::path& path)
{
    using character_type =
        typename rapidjson::Writer<OutputStreamType, SourceEncodingType, TargetEncodingType>::Ch;

    if constexpr (std::is_same_v<character_type, char>) {
        to_json(writer, path.string().c_str());
    } else if (std::is_same_v<character_type, wchar_t>) {
        to_json(writer, path.wstring().c_str());
    }
}
#endif
} // namespace serializer
} // namespace json_utils
