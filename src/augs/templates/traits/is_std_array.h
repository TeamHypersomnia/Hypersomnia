#pragma once
#include <array>

namespace augs {
	template <class T, class _enum>
	class enum_array;
}

template <class T>
struct is_std_array : std::false_type {};

template <class T, std::size_t I>
struct is_std_array<std::array<T, I>> : std::true_type {
	static constexpr auto size = I;
};

template <class T, class _enum>
struct is_std_array<augs::enum_array<T, _enum>> : std::true_type {};

template <class T>
constexpr bool is_std_array_v = is_std_array<T>::value;
