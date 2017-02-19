#pragma once
#include <type_traits>

namespace std {
	template<class... _Types>
	class tuple;

	template<class _Ty1,
		class _Ty2>
		struct pair;
}

template <class T>
struct is_memcpy_safe {
	static const bool value = std::is_trivially_copyable<T>::value;
};

template <>
struct is_memcpy_safe<std::tuple<>> {
	static const bool value = true;
};

template <class... Head>
struct are_types_memcpy_safe;

template <class Head>
struct are_types_memcpy_safe<Head> {
	static constexpr bool value = is_memcpy_safe<Head>::value;
};

template <class Head, class... Tail>
struct are_types_memcpy_safe<Head, Tail...> {
	static constexpr bool value =
		are_types_memcpy_safe<Head>::value
		&&
		are_types_memcpy_safe<Tail...>::value;
};

template <class A>
bool trivial_compare(const A& a, const A& b) {
	static_assert(is_memcpy_safe<A>::value, "Type can't be trivially compared!");
	return !std::memcmp(&a, &b, sizeof(A));
}