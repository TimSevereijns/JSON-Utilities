#pragma once

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>

#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

#include "json_traits.h"

template <typename ContainerType> class base_handler
{
  public:
    virtual bool on_default()
    {
        return true;
    }

    virtual bool on_null()
    {
        return true;
    }

    virtual bool on_bool(bool value)
    {
        return true;
    }

    virtual bool on_int(int value)
    {
        return true;
    }

    virtual bool on_uint(unsigned int value)
    {
        return true;
    }

    virtual bool on_int_64(std::int64_t value)
    {
        return true;
    }

    virtual bool on_uint_64(std::uint64_t value)
    {
        return true;
    }

    virtual bool on_double(double value)
    {
        return true;
    }

    virtual bool on_raw_number(const Ch* value, SizeType length, bool should_copy)
    {
        return true;
    }

    virtual bool on_string(const Ch* value, SizeType length, bool should_copy)
    {
        return true;
    }

    virtual bool on_object_start()
    {
        return true;
    }

    virtual bool on_key(const Ch* value, SizeType length, bool should_copy)
    {
        return true;
    }

    virtual bool on_object_end(SizeType length)
    {
        return true;
    }

    virtual bool on_array_start()
    {
        return true;
    }

    virtual bool on_array_end(SizeType length)
    {
        return true;
    }
};

template <typename ContainerType> class array_handler : public base_handler<ContainerType>
{
  public:
    bool on_default() override
    {
        return true;
    }

    bool on_null() override
    {
        return true;
    }

    bool on_bool(bool value) override
    {
        return true;
    }

    bool on_int(int value) override
    {
        return true;
    }

    bool on_uint(unsigned int value) override
    {
        return true;
    }

    bool on_int_64(std::int64_t value) override
    {
        return true;
    }

    bool on_uint_64(std::uint64_t value) override
    {
        return true;
    }

    bool on_double(double value) override
    {
        return true;
    }

    bool on_raw_number(const Ch* value, SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_string(const Ch* value, SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_object_start() override
    {
        return true;
    }

    bool on_key(const Ch* value, SizeType length, bool should_copy) override
    {
        return true;
    }

    bool on_object_end(SizeType length) override
    {
        return true;
    }

    bool on_array_start() override
    {
        return true;
    }

    bool on_array_end(SizeType length) override
    {
        return true;
    }
};

template <typename ContainerType> struct object_handler
{
    object_handler() : m_state(kExpectObjectStart)
    {
    }

    bool StartObject() override
    {
        switch (m_state) {
            case kExpectObjectStart:
                m_state = kExpectNameOrObjectEnd;
                return true;
            default:
                return false;
        }
    }

    bool String(const char* str, rapidjson::SizeType length, bool) override
    {
        switch (m_state) {
            case kExpectNameOrObjectEnd:
                m_name = std::string(str, length);
                m_state = kExpectValue;
                return true;
            case kExpectValue:
                messages_.insert(MessageMap::value_type(m_name, std::string(str, length)));
                m_state = kExpectNameOrObjectEnd;
                return true;
            default:
                return false;
        }
    }

    bool EndObject(rapidjson::SizeType) override
    {
        return m_state == kExpectNameOrObjectEnd;
    }

    bool Default() override
    {
        return false;
    } // All other events are invalid.

    ContainerType m_container;

    enum State
    {
        kExpectObjectStart,
        kExpectNameOrObjectEnd,
        kExpectValue,
    } m_state;

    std::string m_name;
};

template <typename ContainerType>
class delegating_handler
    : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, delegating_handler>
{
  public:
    bool Default()
    {
        return m_active_handler.Default();
    }

    bool Null()
    {
        return m_active_handler.Null();
    }

    bool Bool(bool value)
    {
        return m_active_handler.Bool(value);
    }

    bool Int(int value)
    {
        return m_active_handler.Int(value);
    }

    bool Uint(unsigned int value)
    {
        return m_active_handler.Uint(value);
    }

    bool Int64(std::int64_t value)
    {
        return m_active_handler.Int64(value);
    }

    bool Uint64(std::uint64_t value)
    {
        return m_active_handler.Uint64(value);
    }

    bool Double(double value)
    {
        return m_active_handler.Double(value);
    }

    bool RawNumber(const Ch* value, SizeType length, bool should_copy)
    {
        return m_active_handler.RawNumber(value, length, should_copy);
    }

    bool String(const Ch* value, SizeType length, bool should_copy)
    {
        return m_active_handler.String(value, length, should_copy);
    }

    bool StartObject()
    {
        return m_active_handler.StartObject();
    }

    bool Key(const Ch* value, SizeType length, bool should_copy)
    {
        return m_active_handler.Key();
    }

    bool EndObject(SizeType length)
    {
        return m_active_handler.EndObject(length);
    }

    bool StartArray()
    {
        return m_active_handler.StartArray();
    }

    bool EndArray(SizeType length)
    {
        return m_active_handler.EndArray(length);
    }

private:
    base_handler<ContainerType> m_active_handler;
};

template <typename ContainerType>
auto dispatch_to_token_handler() -> typename std::enable_if<
    json_utils::traits::treat_as_object_sink<typename ContainerType::value_type>::value>::type
{
}

template <typename ContainerType>
auto dispatch_to_token_handler() -> typename std::enable_if<
    json_utils::traits::treat_as_array_sink<typename ContainerType::value_type>::value>::type
{
}

template <typename ContainerType> void parse_message(const char* json, ContainerType& container)
{
    rapidjson::Reader reader;
    delegating_handler handler;
    rapidjson::StringStream ss(json);
    if (reader.Parse(ss, handler))
        messages.swap(handler.messages_); // Only change it if success.
    else {
        rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
        size_t o = reader.GetErrorOffset();
        std::cout << "Error: " << rapidjson::GetParseError_En(e) << std::endl;
        std::cout << " at offset " << o << " near '" << std::string(json).substr(o, 10) << "...'"
                  << std::endl;
    }
}

namespace json_utils
{
namespace sax_deserializer
{
template <typename ContainerType> auto from_json(const std::string& json)
{
    ContainerType container;
    parse_message(json.data(), container);
}
} // namespace sax_deserializer
} // namespace json_utils