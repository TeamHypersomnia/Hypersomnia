#pragma once
#include <memory>

template <class T>
struct is_unique_ptr : std::false_type {};

template <class... T>
struct is_unique_ptr<std::unique_ptr<T...>> : std::true_type {};

template <class T>
static constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value; 