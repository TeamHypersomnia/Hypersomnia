#pragma once
#include "augs/misc/declare_containers.h"

template <class T>
struct is_constant_size_vector : std::false_type {};

template <class T, unsigned N, bool F, class K>
struct is_constant_size_vector<augs::constant_size_vector<T, N, F, K>> : std::true_type {};

template <class T>
static constexpr bool is_constant_size_vector_v = is_constant_size_vector<T>::value; 