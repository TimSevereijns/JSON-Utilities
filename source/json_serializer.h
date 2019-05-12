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
            serializer::to_json(writer, value);
        }
    } // namespace detail

    namespace serializer
    {
        template <typename Writer> void to_json(Writer& writer, bool data)
        {
            writer.Bool(data);
        }

        template <typename Writer> void to_json(Writer& writer, std::int32_t data)
        {
            writer.Int(data);
        }

        template <typename Writer> void to_json(Writer& writer, std::uint32_t data)
        {
            writer.Uint(data);
        }

        template <typename Writer> void to_json(Writer& writer, std::int64_t data)
        {
            writer.Int64(data);
        }

        template <typename Writer> void to_json(Writer& writer, std::uint64_t data)
        {
            writer.Uint64(data);
        }

        template <typename Writer, typename DataType>
        auto to_json(Writer& writer, DataType data) ->
            typename std::enable_if<std::is_floating_point<DataType>::value>::type
        {
            writer.Double(data);
        }

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char, typename Writer::Ch>::value>::type
        {
            if (data == nullptr) {
                writer.Null();
                return;
            }

            writer.String(data);
        }

        template <typename Writer>
        auto to_json(Writer& writer, const char16_t* const data) ->
            typename std::enable_if<std::is_same<char16_t, typename Writer::Ch>::value>::type
        {
            if (data == nullptr) {
                writer.Null();
                return;
            }

            writer.String(data);
        }

        template <typename Writer>
        auto to_json(Writer& writer, const char32_t* const data) ->
            typename std::enable_if<std::is_same<char32_t, typename Writer::Ch>::value>::type
        {
            if (data == nullptr) {
                writer.Null();
                return;
            }

            writer.String(data);
        }

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::shared_ptr<Type>& pointer)
        {
            if (pointer == nullptr) {
                writer.Null();
                return;
            }

            to_json(writer, *pointer);
        }

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::unique_ptr<Type>& pointer)
        {
            if (pointer == nullptr) {
                writer.Null();
                return;
            }

            to_json(writer, *pointer);
        }

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::weak_ptr<Type>& weakPointer)
        {
            const auto strongPointer = weakPointer.lock();
            if (strongPointer == nullptr) {
                writer.Null();
                return;
            }

            to_json(writer, *strongPointer);
        }

        template <
            typename Writer, typename CharacterType, typename CharacterTraits, typename Allocator>
        void to_json(
            Writer& writer,
            const std::basic_string<CharacterType, CharacterTraits, Allocator>& data)
        {
            static_assert(
                std::is_same<
                    typename std::basic_string<
                        CharacterType, CharacterTraits, Allocator>::value_type,
                    typename Writer::Ch>::value,
                "The character type to be serialized differs from the character type of the "
                "rapidjson::Writer object.");

            writer.String(data.c_str());
        }

        // @todo Add overload for a vector of pairs

        template <typename Writer, typename Type>
        auto to_json(Writer& writer, const Type& container) ->
            typename std::enable_if<traits::treat_as_array<Type>::value>::type
        {
            writer.StartArray();
            for (const auto& item : container) {
                to_json(writer, item);
            }
            writer.EndArray();
        }

        template <typename Writer, typename Type>
        auto to_json(Writer& writer, const Type& container) ->
            typename std::enable_if<traits::treat_as_object<Type>::value>::type
        {
            writer.StartObject();
            for (const auto& item : container) {
                to_json(writer, item);
            }
            writer.EndObject();
        }

        template <typename Writer, typename FirstType, typename SecondType>
        void to_json(Writer& writer, const std::pair<FirstType, SecondType>& pair)
        {
            detail::insert_key_value_pair(writer, pair.first, pair.second);
        }

#if __cplusplus >= 201703L // C++17
        template <typename Writer, typename CharacterType, typename CharacterTraits>
        void
        to_json(Writer& writer, const std::basic_string_view<CharacterType, CharacterTraits>& view)
        {
            writer.String(view.data());
        }

        template <typename Writer, typename DataType>
        void to_json(Writer& writer, const std::optional<DataType>& data)
        {
            if (!data.has_value()) {
                writer.Null();
                return;
            }

            to_json(writer, *data);
        }

        template <typename Writer> void to_json(Writer& writer, const std::filesystem::path& path)
        {
            if constexpr (std::is_same_v<typename Writer::Ch, char>) {
                to_json(writer, path.string().c_str());
            } else if (std::is_same_v<typename Writer::Ch, wchar_t>) {
                to_json(writer, path.wstring().c_str());
            }
        }
#endif
    } // namespace serializer
} // namespace json_utils
