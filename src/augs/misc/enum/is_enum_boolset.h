#pragma once
#include <cstddef>

namespace augs {
	template <class _enum, std::size_t alignment>
	class enum_boolset;
}

template <class T>
struct is_enum_boolset : std::false_type {};

template <class A, std::size_t B>
struct is_enum_boolset<augs::enum_boolset<A, B>> : std::true_type {};

template <class T>
static constexpr bool is_enum_boolset_v = is_enum_boolset<T>::value; 
