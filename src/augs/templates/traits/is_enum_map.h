#pragma once
#include <cstddef>
#include "augs/misc/declare_containers.h"

template <class T>
struct is_enum_map : std::false_type {};

template <class _enum, class T>
struct is_enum_map<augs::enum_map<_enum, T>> : std::true_type {
	static constexpr std::size_t size = static_cast<std::size_t>(_enum::COUNT);
};

template <class T>
constexpr bool is_enum_map_v = is_enum_map<T>::value;

