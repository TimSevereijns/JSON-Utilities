#pragma once

#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace json_utils
{
namespace traits
{
template <typename...> using void_t = void;

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
    Type,
    void_t<decltype(std::declval<Type&>().emplace(std::declval<typename Type::value_type&&>()))>>
    : std::true_type
{
};

template <typename, typename = void> struct treat_as_array : std::false_type
{
};

template <typename Type>
struct treat_as_array<
    Type, void_t<
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
} // namespace json_utils
