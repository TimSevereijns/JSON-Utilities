#pragma once

#if __cplusplus >= 201703L

#include <filesystem>
#include <fstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#endif

#include "json_dom_deserializer.h"
#include "json_sax_deserializer.h"
#include "json_serializer.h"

namespace json_utils
{
namespace detail
{
template <
    typename ContainerType, typename EncodingType,
    unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags, typename StreamType>
ContainerType deserialize(StreamType& stream)
{
    rapidjson::GenericDocument<EncodingType> document;
    document.template ParseStream<ParseFlags>(stream);

    if (RAPIDJSON_UNLIKELY(document.HasParseError())) {
        throw std::invalid_argument{ "Could not parse JSON document." };
    }

    static_assert(
        std::is_default_constructible<ContainerType>::value,
        "The container must have a default constructor.");

    ContainerType container;
    dom_deserializer::from_json(document, container);

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

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_dom(const char* const json)
{
    using encoding_type = rapidjson::UTF8<>;

    rapidjson::GenericStringStream<encoding_type> string_stream{ json };
    return detail::deserialize<ContainerType, encoding_type, ParseFlags>(string_stream);
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_dom(const std::string& json)
{
    return deserialize_via_dom<ContainerType, ParseFlags>(json.c_str());
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_dom(const wchar_t* const json)
{
    using encoding_type = rapidjson::UTF16<>;

    rapidjson::GenericStringStream<encoding_type> string_stream{ json };
    return detail::deserialize<ContainerType, encoding_type, ParseFlags>(string_stream);
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_dom(const std::wstring& json)
{
    return deserialize_via_dom<ContainerType, ParseFlags>(json.c_str());
}

#if __cplusplus >= 201703L

template <
    typename InputEncodingType = rapidjson::UTF8<>, typename OutputEncodingType = rapidjson::UTF8<>,
    typename DataType>
void serialize_to_json(const DataType& data, const std::filesystem::path& path)
{
    std::ofstream file_stream{ path };
    rapidjson::OStreamWrapper stream_wrapper{ file_stream };
    rapidjson::Writer<decltype(stream_wrapper), InputEncodingType, OutputEncodingType> writer{
        stream_wrapper
    };

    serializer::to_json(writer, data);
}

template <
    typename InputEncodingType = rapidjson::UTF8<>, typename OutputEncodingType = rapidjson::UTF8<>,
    typename DataType>
void serialize_to_pretty_json(const DataType& data, const std::filesystem::path& path)
{
    std::ofstream file_stream{ path };
    rapidjson::OStreamWrapper stream_wrapper{ file_stream };
    rapidjson::PrettyWriter<decltype(stream_wrapper), InputEncodingType, OutputEncodingType> writer{
        stream_wrapper
    };

    serializer::to_json(writer, data);
}

template <
    typename ContainerType, typename EncodingType = rapidjson::UTF8<>,
    unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_dom(const std::filesystem::path& path)
{
    std::ifstream file_stream{ path };
    rapidjson::IStreamWrapper stream_wrapper{ file_stream };

    return detail::deserialize<ContainerType, EncodingType, ParseFlags>(stream_wrapper);
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_sax(const char* const json)
{
    return sax_deserializer::detail::from_json<ContainerType, ParseFlags>(json);
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_sax(const std::string& json)
{
    return sax_deserializer::detail::from_json<ContainerType, ParseFlags>(json.c_str());
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_sax(const wchar_t* const json)
{
    return sax_deserializer::detail::from_json<ContainerType, ParseFlags>(json);
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_sax(const std::wstring& json)
{
    return sax_deserializer::detail::from_json<ContainerType, ParseFlags>(json.c_str());
}

template <
    typename ContainerType, unsigned int ParseFlags = rapidjson::ParseFlag::kParseDefaultFlags>
JSON_UTILS_NODISCARD ContainerType deserialize_via_sax(const std::filesystem::path& path)
{
    return sax_deserializer::detail::from_json<ContainerType, ParseFlags>(path);
}

#endif
} // namespace json_utils
