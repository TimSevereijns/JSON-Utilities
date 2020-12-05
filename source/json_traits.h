#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#if __cplusplus >= 201703L
#include <optional>
#endif

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

template <typename DataType> struct treat_as_array_or_object_sink
{
    static constexpr bool value = !treat_as_value_sink<DataType>::value;
};

template <typename, typename = void> struct is_shared_ptr : std::false_type
{
};

template <typename ElementType> struct is_shared_ptr<std::shared_ptr<ElementType>> : std::true_type
{
};

template <typename, typename = void> struct is_unique_ptr : std::false_type
{
};

template <typename ElementType> struct is_unique_ptr<std::unique_ptr<ElementType>> : std::true_type
{
};

#if __cplusplus >= 201703L

template <typename, typename = void> struct is_optional : std::false_type
{
};

template <typename ElementType> struct is_optional<std::optional<ElementType>> : std::true_type
{
};

template <typename ContainerType>
constexpr bool has_emplace_back_v = has_emplace_back<ContainerType>::value;

template <typename ContainerType>
constexpr bool has_emplace_v = has_emplace<ContainerType>::value;

template <typename Type>
constexpr bool is_container_v = is_container<Type>::value;

template <typename Type>
constexpr bool is_pair_v = is_pair<Type>::value;

template <typename ContainerType>
constexpr bool treat_as_array_sink_v = treat_as_array_sink<ContainerType>::value;

template <typename ContainerType>
constexpr bool treat_as_object_sink_v = treat_as_object_sink<ContainerType>::value;

template <typename ContainerType>
constexpr bool treat_as_value_sink_v = treat_as_value_sink<ContainerType>::value;

template <typename ContainerType>
constexpr bool treat_as_array_or_object_sink_v = treat_as_array_or_object_sink<ContainerType>::value;

template <typename Type>
constexpr bool is_shared_ptr_v = is_shared_ptr<Type>::value;

template <typename Type>
constexpr bool is_unique_ptr_v = is_unique_ptr<Type>::value;

template <typename Type>
constexpr bool is_optional_v = is_optional<Type>::value;

#endif
} // namespace traits
} // namespace json_utils
