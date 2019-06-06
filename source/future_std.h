#pragma once

#include <type_traits>

/**
 * The functionality provided in this namespace emulates similar functionality found in C++17.
 */
namespace future_std
{
template <typename...> using void_t = void;

template <typename...> struct conjunction : std::true_type
{
};

template <typename DataType> struct conjunction<DataType> : DataType
{
};

template <typename DataType, typename... OtherDataTypes>
struct conjunction<DataType, OtherDataTypes...>
    : std::conditional<bool(DataType::value), conjunction<OtherDataTypes...>, DataType>::type
{
};

template <typename...> struct disjunction : std::true_type
{
};

template <typename DataType> struct disjunction<DataType> : DataType
{
};

template <class DataType, class... OtherDataTypes>
struct disjunction<DataType, OtherDataTypes...>
    : std::conditional<bool(DataType::value), DataType, disjunction<OtherDataTypes...>>::type
{
};
} // namespace future_std
