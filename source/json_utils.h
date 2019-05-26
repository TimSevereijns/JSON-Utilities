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

    template <typename ContainerType> ContainerType deserialize_from_json(const std::string& json)
    {
        rapidjson::StringStream stringStream{ json.c_str() };
        rapidjson::Document document;
        document.ParseStream(stringStream);

        static_assert(
            std::is_default_constructible<ContainerType>::value,
            "The container must have a default constructor defined.");

        ContainerType container;

        using deserializer::from_json; //< Enables ADL
        from_json(document, container);

        return container;
    }

    template <typename ContainerType> ContainerType deserialize_from_json(const char* const json)
    {
        rapidjson::StringStream stringStream{ json };
        rapidjson::Document document;
        document.ParseStream(stringStream);

        static_assert(
            std::is_default_constructible<ContainerType>::value,
            "The container must have a default constructor defined.");

        ContainerType container;

        using deserializer::from_json; //< Enables ADL
        from_json(document, container);

        return container;
    }
} // namespace json_utils
