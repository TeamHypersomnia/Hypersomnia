#pragma once
#include <variant>

template <class T>
struct is_variant : std::false_type {};

template <class... T>
struct is_variant<std::variant<T...>> : std::true_type {};

template <class T>
static constexpr bool is_variant_v = is_variant<T>::value; 