#pragma once
#include <type_traits>
#include "augs/misc/is_constant_size_string.h"

#define LUA_TRAITS_INCLUDED 1

namespace augs {
	template <class T, class = void>
	struct has_custom_to_lua_value : std::false_type {};

	template <class T>
	struct has_custom_to_lua_value<
		T, 
		decltype(
			to_lua_value(
				std::declval<const T&>()
			),
			from_lua_value(
				std::declval<int&>(),
				std::declval<T&>()
			),
			void()
		)
	> : std::true_type {};

	template <class T>
	constexpr bool has_custom_to_lua_value_v = has_custom_to_lua_value<T>::value;

	template <class T>
	constexpr bool representable_as_lua_value_v =
		std::is_same_v<T, std::string>
		|| is_constant_size_string_v<T>
		|| std::is_arithmetic_v<T>
		|| std::is_enum_v<T>
		|| has_custom_to_lua_value_v<T>
	;
	
	template <class T>
	constexpr bool representable_as_lua_value_v<T*> = representable_as_lua_value_v<std::remove_const_t<T>>;

	template <class T, class = void>
	struct key_representable_as_lua_value : std::false_type {};

	template <class T>
	struct key_representable_as_lua_value<T, decltype(typename T::key_type(), void())> 
		: std::bool_constant<representable_as_lua_value_v<typename T::key_type>> {
	};

	template <class T>
	static constexpr bool key_representable_as_lua_value_v = key_representable_as_lua_value<T>::value;
}