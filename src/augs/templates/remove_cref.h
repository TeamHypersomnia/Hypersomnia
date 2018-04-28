#pragma once
#include <type_traits>

template <class T>
using remove_cref = std::remove_const_t<std::remove_reference_t<T>>;