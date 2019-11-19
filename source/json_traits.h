#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include "future_std.h"

namespace json_utils
{
namespace traits
{
template <typename, typename = void> struct has_emplace_back : std::false_type
{
};

template <typename Type>
struct has_emplace_back<
    Type, future_std::void_t<decltype(std::declval<Type&>().emplace_back(
              std::declval<typename Type::value_type&&>()))>> : std::true_type
{
};

template <typename, typename = void> struct has_emplace : std::false_type
{
};

template <typename Type>
struct has_emplace<
    Type, future_std::void_t<decltype(std::declval<Type&>().emplace(
              std::declval<typename Type::value_type&&>()))>> : std::true_type
{
};

template <typename, typename = void> struct is_container : std::false_type
{
};

template <typename Type>
struct is_container<
    Type, future_std::void_t<
              typename Type::value_type, typename Type::iterator,
              decltype(std::declval<Type&>().begin()), decltype(std::declval<Type&>().end())>>
    : std::true_type
{
};

template <typename> struct is_pair : std::false_type
{
};

template <typename FirstType, typename SecondType>
struct is_pair<std::pair<FirstType, SecondType>> : std::true_type
{
};

template <typename, typename = void> struct treat_as_array_sink : std::false_type
{
};

/**
 * @note Uses SFINAE to detect whether the input type has a `value_type` definition, and if it does,
 * additional type traits will determine whether we're dealing with a container whose value type is
 * something other than a `std::pair<...>`.
 **/
template <typename DataType>
struct treat_as_array_sink<DataType, future_std::void_t<typename DataType::value_type>>
    : std::conditional<
          is_container<DataType>::value && !is_pair<typename DataType::value_type>::value,
          std::true_type, std::false_type>::type
{
};

template <typename ArrayType, std::size_t ArraySize>
struct treat_as_array_sink<ArrayType[ArraySize]> : std::true_type
{
};

template <std::size_t ArraySize> struct treat_as_array_sink<char[ArraySize]> : std::false_type
{
};

template <std::size_t ArraySize> struct treat_as_array_sink<wchar_t[ArraySize]> : std::false_type
{
};

template <typename CharacterType, typename CharacterTraitsType, typename AllocatorType>
struct treat_as_array_sink<std::basic_string<CharacterType, CharacterTraitsType, AllocatorType>>
    : std::false_type
{
};

template <typename, typename = void> struct treat_as_object_sink : std::false_type
{
};

/**
 * @note Uses SFINAE to detect whether the input type has a `value_type` definition, and if it does,
 * additional type traits will determine whether we're dealing with a container whose value type is
 * a `std::pair<...>`. Anything that stores a `std::pair<...>` will therefore be treated as an
 * acceptable sink for a JSON object.
 **/
template <typename DataType>
struct treat_as_object_sink<DataType, future_std::void_t<typename DataType::value_type>>
    : std::conditional<
          is_container<DataType>::value && is_pair<typename DataType::value_type>::value,
          std::true_type, std::false_type>::type
{
};

template <typename DataType> struct treat_as_value_sink
{
    static constexpr bool value =
        !(treat_as_array_sink<DataType>::value || treat_as_object_sink<DataType>::value);
};
} // namespace traits
} // namespace json_utils
