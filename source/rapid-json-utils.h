#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>

namespace json_utils
{
    namespace traits
    {
        template <typename... SFINAE> using void_t = void;

        template <typename, typename = void> struct has_emplace_back : std::false_type
        {
        };

        template <typename Type>
        struct has_emplace_back<
            Type, void_t<decltype(std::declval<Type&>().emplace_back(
                      std::declval<typename Type::value_type&&>()))>> : std::true_type
        {
        };

        template <typename, typename = void> struct has_emplace : std::false_type
        {
        };

        template <typename Type>
        struct has_emplace<
            Type, void_t<decltype(std::declval<Type&>().emplace(
                      std::declval<typename Type::value_type&&>()))>> : std::true_type
        {
        };

        template <typename, typename = void> struct treat_as_array : std::false_type
        {
        };

        template <typename Type>
        struct treat_as_array<
            Type,
            void_t<
                typename Type::value_type, typename Type::reference, typename Type::const_reference,
                typename Type::iterator, decltype(std::declval<Type&>().begin()),
                decltype(std::declval<Type&>().end())>> : std::true_type
        {
        };

        template <typename... Args> struct treat_as_array<std::tuple<Args...>> : std::true_type
        {
        };

        template <typename ArrayType, std::size_t ArraySize>
        struct treat_as_array<ArrayType[ArraySize]> : std::true_type
        {
        };

        template <std::size_t ArraySize> struct treat_as_array<char[ArraySize]> : std::false_type
        {
        };

        template <std::size_t ArraySize> struct treat_as_array<wchar_t[ArraySize]> : std::false_type
        {
        };

        template <typename CharacterType, typename CharacterTraitsType, typename AllocatorType>
        struct treat_as_array<std::basic_string<CharacterType, CharacterTraitsType, AllocatorType>>
            : std::false_type
        {
        };

        template <typename KeyType, typename ValueType>
        struct treat_as_array<std::map<KeyType, ValueType>> : std::false_type
        {
        };

        template <typename KeyType, typename ValueType>
        struct treat_as_array<std::unordered_map<KeyType, ValueType>> : std::false_type
        {
        };

        // @todo Generalize to handle any array-type of pairs.
        template <typename KeyType, typename ValueType>
        struct treat_as_array<std::vector<std::pair<KeyType, ValueType>>> : std::false_type
        {
        };

        template <typename, typename = void> struct treat_as_object : std::false_type
        {
        };

        template <typename KeyType, typename ValueType>
        struct treat_as_object<std::map<KeyType, ValueType>> : std::true_type
        {
        };

        template <typename KeyType, typename ValueType>
        struct treat_as_object<std::unordered_map<KeyType, ValueType>> : std::true_type
        {
        };

        template <typename KeyType, typename ValueType>
        struct treat_as_object<std::vector<std::pair<KeyType, ValueType>>> : std::true_type
        {
        };
    } // namespace traits

    namespace serializer
    {
        template <typename Writer> void to_json(Writer& writer, bool data);

        template <typename Writer> void to_json(Writer& writer, std::int32_t data);

        template <typename Writer> void to_json(Writer& writer, std::uint32_t data);

        template <typename Writer> void to_json(Writer& writer, std::int64_t data);

        template <typename Writer> void to_json(Writer& writer, std::uint64_t data);

        template <typename Writer, typename DataType>
        auto to_json(Writer& writer, DataType data) ->
            typename std::enable_if<std::is_floating_point<DataType>::value>::type;

        template <
            typename Writer, typename CharacterType, typename CharacterTraits, typename Allocator>
        void to_json(
            Writer& writer,
            const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char, typename Writer::Ch>::value>::type;

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char16_t, typename Writer::Ch>::value>::type;

        template <typename Writer>
        auto to_json(Writer& writer, const char* const data) ->
            typename std::enable_if<std::is_same<char32_t, typename Writer::Ch>::value>::type;

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::shared_ptr<Type>& pointer);

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::unique_ptr<Type>& pointer);

        template <typename Writer, typename Type>
        void to_json(Writer& writer, const std::weak_ptr<Type>& pointer);

        template <
            typename Writer, typename CharacterType, typename CharacterTraits, typename Allocator>
        void to_json(
            Writer& writer,
            const std::basic_string<CharacterType, CharacterTraits, Allocator>& data);

        template <typename Writer, typename Type>
        auto to_json(Writer& writer, const Type& container) ->
            typename std::enable_if<traits::treat_as_array<Type>::value>::type;

        template <typename Writer, typename Type>
        auto to_json(Writer& writer, const Type& container) ->
            typename std::enable_if<traits::treat_as_object<Type>::value>::type;

        template <typename Writer, typename FirstType, typename SecondType>
        void to_json(Writer& writer, const std::pair<FirstType, SecondType>& pair);

#if __cplusplus >= 201703L // C++17
        template <typename Writer, typename CharacterType, typename CharacterTraits>
        void
        to_json(Writer& writer, const std::basic_string_view<CharacterType, CharacterTraits>& view);

        template <typename Writer, typename DataType>
        void to_json(Writer& writer, const std::optional<DataType>& data);

        template <typename Writer> void to_json(Writer& writer, const std::filesystem::path& path);
#endif
    } // namespace serializer

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
    }; // namespace serializer

    namespace deserializer
    {
        struct back_inserter_policy
        {
            template <typename DataType, typename ContainerType>
            static void insert(DataType&& data, ContainerType& container)
            {
                container.emplace_back(std::forward<DataType>(data));
            }
        };

        struct inserter_policy
        {
            template <typename DataType, typename ContainerType>
            static void insert(DataType&& data, ContainerType& container)
            {
                container.emplace(std::forward<DataType>(data));
            }
        };

        template <typename InsertionPolicy, typename DataType, typename ContainerType>
        void insert(DataType&& value, ContainerType& container)
        {
            static_assert(
                std::disjunction<
                    std::is_same<DataType, typename ContainerType::value_type>,
                    std::is_convertible<DataType, typename ContainerType::value_type>>::value,
                "The type being inserted is not the same as, or cannot be converted to, the "
                "container's value type.");

            InsertionPolicy::insert(std::forward<DataType>(value), container);
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<int, typename ContainerType::value_type>::value>::type
        {
            if (value.IsInt()) {
                insert<InsertionPolicy>(value.GetInt(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<unsigned int, typename ContainerType::value_type>::value>::type
        {
            if (value.IsUint()) {
                insert<InsertionPolicy>(value.GetUint(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<std::int64_t, typename ContainerType::value_type>::value>::type
        {
            if (value.IsInt64()) {
                insert<InsertionPolicy>(value.GetInt64(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<std::uint64_t, typename ContainerType::value_type>::value>::type
        {
            if (value.IsUint64()) {
                insert<InsertionPolicy>(value.GetUint64(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<std::string, typename ContainerType::value_type>::value>::type
        {
            if (value.IsString()) {
                insert<InsertionPolicy>(value.GetString(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                std::is_same<double, typename ContainerType::value_type>::value>::type
        {
            if (value.IsDouble()) {
                insert<InsertionPolicy>(value.GetDouble(), container);
            } else {
                assert(false);
            }
        }

        template <typename InsertionPolicy, typename ContainerType, typename DataType>
        auto from_json_array(const rapidjson::GenericArray<true, DataType>& jsonArray)
        {
            ContainerType container{};

            for (const auto& jsonValue : jsonArray) {
                dispatch_insertion<InsertionPolicy>(jsonValue, container);
            }

            return container;
        }

        template <typename InsertionPolicy, typename ContainerType>
        auto dispatch_insertion(const rapidjson::Value& value, ContainerType& container) ->
            typename std::enable_if<
                traits::treat_as_array<typename ContainerType::value_type>::value>::type
        {
            using nested_container_type = typename ContainerType::value_type;

            if (value.IsArray()) {
                auto nestedContainer =
                    from_json_array<InsertionPolicy, nested_container_type>(value.GetArray());
                insert<InsertionPolicy>(std::move(nestedContainer), container);
            } else {
                assert(false);
            }
        }

        template <typename ContainerType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename ContainerType::second_type, int>::value,
                std::pair<std::string, int>>::type
        {
            return std::make_pair<std::string, int>(member.name.GetString(), member.value.GetInt());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::uint32_t>::value,
                std::pair<std::string, std::uint32_t>>::type
        {
            return std::make_pair<std::string, std::uint32_t>(
                member.name.GetString(), member.value.GetUint());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::int64_t>::value,
                std::pair<std::string, std::int64_t>>::type
        {
            return std::make_pair<std::string, std::int64_t>(
                member.name.GetString(), member.value.GetInt64());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, std::uint64_t>::value,
                std::pair<std::string, std::uint64_t>>::type
        {
            return std::make_pair<std::string, std::uint64_t>(
                member.name.GetString(), member.value.GetUint64());
        }

        template <typename PairType, typename EncodingType, typename AllocatorType>
        auto to_key_value_pair(const rapidjson::GenericMember<EncodingType, AllocatorType>& member)
            -> typename std::enable_if<
                std::is_same<typename PairType::second_type, double>::value,
                std::pair<std::string, double>>::type
        {
            return std::make_pair<std::string, double>(
                member.name.GetString(), member.value.GetDouble());
        }

        template <
            typename InsertionPolicy, typename EncodingType, typename AllocatorType,
            typename ContainerType>
        auto dispatch_insertion(
            const rapidjson::GenericMember<EncodingType, AllocatorType>& member,
            ContainerType& container)
        {
            auto pair = to_key_value_pair<typename ContainerType::value_type>(member);
            insert<InsertionPolicy>(std::move(pair), container);
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_array<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container{};

            if (document.IsArray()) {
                for (const auto& jsonValue : document.GetArray()) {
                    dispatch_insertion<back_inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace_back<ContainerType>,
                traits::treat_as_object<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container{};

            if (document.IsObject()) {
                for (const auto& jsonValue : document.GetObject()) {
                    dispatch_insertion<back_inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }

        template <typename ContainerType>
        auto from_json(const rapidjson::Document& document) -> typename std::enable_if<
            std::conjunction<
                traits::has_emplace<ContainerType>, traits::treat_as_object<ContainerType>>::value,
            ContainerType>::type
        {
            ContainerType container{};

            if (document.IsObject()) {
                for (const auto& jsonValue : document.GetObject()) {
                    dispatch_insertion<inserter_policy>(jsonValue, container);
                }
            }

            return container;
        }
    } // namespace deserializer

    template <typename EncodingType = rapidjson::UTF8<>, typename DataType>
    std::basic_string<typename EncodingType::Ch> serialize_to_pretty_json(const DataType& data)
    {
        rapidjson::GenericStringBuffer<EncodingType> buffer;
        rapidjson::PrettyWriter<decltype(buffer)> writer{ buffer };

        using serializer::to_json;
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