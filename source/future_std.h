#pragma once

#include <type_traits>

// clang-format off
#if __cplusplus >= 201703L // C++17
    #ifndef JSON_UTILS_NORETURN
        #define JSON_UTILS_NORETURN [[noreturn]]
    #endif

    #ifndef JSON_UTILS_NODISCARD
        #define JSON_UTILS_NODISCARD [[nodiscard]]
    #endif

    #ifndef JSON_UTILS_MAYBEUNUSED
        #define JSON_UTILS_MAYBE_UNUSED [[maybe_unused]]
    #endif
#else
    #ifndef JSON_UTILS_NORETURN
        #define JSON_UTILS_NORETURN
    #endif

    #ifndef JSON_UTILS_NODISCARD
        #define JSON_UTILS_NODISCARD
    #endif

    #ifndef JSON_UTILS_MAYBEUNUSED
        #define JSON_UTILS_MAYBEUNUSED
    #endif
#endif
// clang-format on

/**
 * The functionality provided in this namespace emulates similar functionality found in C++14 and
 * C++17.
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

template <class DataType> struct negation : std::integral_constant<bool, !bool(DataType::value)>
{
};
} // namespace future_std
