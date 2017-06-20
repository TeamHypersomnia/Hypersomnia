#pragma once
#include <optional>

template <class T>
struct is_optional : std::false_type {};

template <class T>
struct is_optional<std::optional<T>> : std::true_type {};

template <class T>
static constexpr bool is_optional_v = is_optional<T>::value;