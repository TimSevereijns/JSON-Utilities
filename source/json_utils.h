#pragma once

#if __cplusplus >= 201703L

#include <filesystem>
#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#endif

#include "json_deserializer.h"
#include "json_serializer.h"

namespace json_utils
{
namespace detail
{
template <typename ContainerType, typename StreamType> ContainerType deserialize(StreamType& stream)
{
    rapidjson::Document document;
    document.ParseStream(stream);

    if (document.HasParseError()) {
        throw std::invalid_argument("Could not parse JSON document.");
    }

    static_assert(
        std::is_default_constructible<ContainerType>::value,
        "The container must have a default constructible.");

    ContainerType container;
    deserializer::from_json(document, container);

    return container;
}
} // namespace detail

template <
    typename InputEncodingType = rapidjson::UTF8<>, typename OutputEncodingType = rapidjson::UTF8<>,
    typename DataType>
JSON_UTILS_NODISCARD std::basic_string<typename OutputEncodingType::Ch>
serialize_to_json(const DataType& data)
{
    rapidjson::GenericStringBuffer<OutputEncodingType> buffer;
    rapidjson::Writer<decltype(buffer), InputEncodingType, OutputEncodingType> writer{ buffer };

    serializer::to_json(writer, data);
    return buffer.GetString();
}

template <
    typename InputEncodingType = rapidjson::UTF8<>, typename OutputEncodingType = rapidjson::UTF8<>,
    typename DataType>
JSON_UTILS_NODISCARD std::basic_string<typename OutputEncodingType::Ch>
serialize_to_pretty_json(const DataType& data)
{
    rapidjson::GenericStringBuffer<OutputEncodingType> buffer;
    rapidjson::PrettyWriter<decltype(buffer), InputEncodingType, OutputEncodingType> writer{
        buffer
    };

    serializer::to_json(writer, data);

    return buffer.GetString();
}

template <typename ContainerType>
JSON_UTILS_NODISCARD ContainerType deserialize_from_json(const char* const json)
{
    rapidjson::StringStream string_stream{ json };
    rapidjson::Document document;

    return detail::deserialize<ContainerType>(string_stream);
}

template <typename ContainerType>
JSON_UTILS_NODISCARD ContainerType deserialize_from_json(const std::string& json)
{
    return deserialize_from_json<ContainerType>(json.c_str());
}

#if __cplusplus >= 201703L

template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
void serialize_to_json(const DataType& data, const std::filesystem::path& path)
{
    std::ofstream file_stream{ path };

    rapidjson::OStreamWrapper stream_wrapper{ file_stream };
    rapidjson::Writer<decltype(stream_wrapper)> writer{ stream_wrapper };

    serializer::to_json(writer, data);
}

template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
void serialize_to_pretty_json(const DataType& data, const std::filesystem::path& path)
{
    std::ofstream file_stream{ path };

    rapidjson::OStreamWrapper stream_wrapper{ file_stream };
    rapidjson::PrettyWriter<decltype(stream_wrapper)> writer{ stream_wrapper };

    serializer::to_json(writer, data);
}

template <typename ContainerType>
JSON_UTILS_NODISCARD ContainerType deserialize_from_json(const std::filesystem::path& path)
{
    std::ifstream file_stream{ path };
    rapidjson::IStreamWrapper stream_wrapper{ file_stream };

    return detail::deserialize<ContainerType>(stream_wrapper);
}

#endif
} // namespace json_utils
