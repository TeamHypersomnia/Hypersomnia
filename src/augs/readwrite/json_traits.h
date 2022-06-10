#pragma once
#include <type_traits>
#include "augs/misc/is_constant_size_string.h"

#define JSON_TRAITS_INCLUDED 1

namespace augs {
	template <class T, class = void>
	struct has_custom_to_json_value : std::false_type {};

	template <class T>
	struct has_custom_to_json_value<
		T, 
		decltype(
			to_json_value(
				std::declval<int&>(),
				std::declval<const T&>()
			),
			from_json_value(
				std::declval<int&>(),
				std::declval<T&>()
			),
			void()
		)
	> : std::true_type {};

	template <class T>
	constexpr bool has_custom_to_json_value_v = has_custom_to_json_value<T>::value;

	template <class T>
	constexpr bool representable_as_json_value_v =
		std::is_same_v<T, std::string>
		|| is_constant_size_string_v<T>
		|| std::is_arithmetic_v<T>
		|| std::is_enum_v<T>
		|| has_custom_to_json_value_v<T>
	;
	
	template <class T>
	constexpr bool representable_as_json_value_v<T*> = representable_as_json_value_v<std::remove_const_t<T>>;

	template <class T, class = void>
	struct key_representable_as_string : std::false_type {};

	template <class T>
	struct key_representable_as_string<T, decltype(typename T::key_type(), void())> 
		: std::bool_constant<std::is_same_v<std::string, typename T::key_type>> {
	};

	template <class T>
	static constexpr bool key_representable_as_string_v = key_representable_as_string<T>::value;


	template <class T, class = void>
	struct json_ignore : std::false_type {};

	template <class T>
	struct json_ignore<
		T, 
		decltype(
			T::json_ignore,
			void()
		)
	> : std::bool_constant<T::json_ignore> {};

	template <class T>
	constexpr bool json_ignore_v = json_ignore<remove_cref<T>>::value;
}