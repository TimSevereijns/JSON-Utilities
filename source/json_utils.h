#pragma once

#include "json_deserializer.h"
#include "json_serializer.h"

namespace json_utils
{
namespace detail
{
template <typename WriterType, typename DataType, typename BufferType>
const typename BufferType::Ch*
serialize(BufferType& buffer, WriterType& writer, const DataType& data)
{
    using serializer::to_json;
    to_json(writer, data);

    return buffer.GetString();
}
} // namespace detail

template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
std::basic_string<typename EncodingType::Ch> serialize_to_json(const DataType& data)
{
    rapidjson::GenericStringBuffer<EncodingType> buffer;
    rapidjson::Writer<decltype(buffer)> writer{ buffer };

    return detail::serialize(buffer, writer, data);
}

template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
std::basic_string<typename EncodingType::Ch> serialize_to_pretty_json(const DataType& data)
{
    rapidjson::GenericStringBuffer<EncodingType> buffer;
    rapidjson::PrettyWriter<decltype(buffer)> writer{ buffer };

    return detail::serialize(buffer, writer, data);
}

template <typename ContainerType> ContainerType deserialize_from_json(const char* const json)
{
    rapidjson::StringStream stringStream{ json };
    rapidjson::Document document;
    document.ParseStream(stringStream);

    if (document.HasParseError()) {
        throw std::invalid_argument("Could not parse JSON document.");
    }

    static_assert(
        std::is_default_constructible<ContainerType>::value,
        "The container must have a default constructible.");

    ContainerType container;

    using deserializer::from_json;
    from_json(document, container);

    return container;
}

template <typename ContainerType> ContainerType deserialize_from_json(const std::string& json)
{
    return deserialize_from_json<ContainerType>(json.c_str());
}
} // namespace json_utils
