#pragma once

#include "json_deserializer.h"
#include "json_serializer.h"

namespace json_utils
{
    template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
    std::basic_string<typename EncodingType::Ch> serialize_to_json(const DataType& data)
    {
        rapidjson::GenericStringBuffer<EncodingType> buffer;
        rapidjson::Writer<decltype(buffer)> writer{ buffer };

        using serializer::to_json; //< Enables ADL
        to_json(writer, data);

        return buffer.GetString();
    }

    template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
    std::basic_string<typename EncodingType::Ch> serialize_to_pretty_json(const DataType& data)
    {
        rapidjson::GenericStringBuffer<EncodingType> buffer;
        rapidjson::PrettyWriter<decltype(buffer)> writer{ buffer };

        using serializer::to_json; //< Enables ADL
        to_json(writer, data);

        return buffer.GetString();
    }

    template <typename ContainerType, typename EncodingType = rapidjson::UTF8<>>
    ContainerType deserialize_from_json(const std::string& json)
    {
        rapidjson::StringStream stringStream{ json.c_str() };
        rapidjson::Document document;
        document.ParseStream(stringStream);

        auto container = deserializer::from_json<ContainerType>(document);

        return container;
    }
} // namespace json_utils
