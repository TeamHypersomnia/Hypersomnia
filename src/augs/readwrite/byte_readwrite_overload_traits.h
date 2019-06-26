#pragma once
#include <type_traits>
#include "augs/templates/type_list.h"
#include "augs/readwrite/special_readwrite_traits.h"

#define READWRITE_OVERLOAD_TRAITS_INCLUDED 1

namespace augs {
	template <class ArgsList, class = void>
	struct has_byte_read_overload : std::false_type 
	{};

	template <class... Args>
	struct has_byte_read_overload <
		type_list<Args...>,
		decltype(read_object_bytes(std::declval<Args&>()...), void())
	> : std::true_type 
	{};

	template <class ArgsList, class = void>
	struct has_byte_write_overload : std::false_type 
	{};

	template <class... Args>
	struct has_byte_write_overload <
		type_list<Args...>,
		decltype(write_object_bytes(std::declval<Args&>()...), void())
	> : std::true_type 
	{};

	template <class... Args>
	constexpr bool has_byte_read_overload_v = has_byte_read_overload<type_list<Args...>>::value;

	template <class... Args>
	constexpr bool has_byte_write_overload_v = has_byte_write_overload<type_list<Args...>>::value;

	template <class... Args>
	constexpr bool has_byte_readwrite_overloads_v = 
		has_byte_read_overload_v<Args...> && has_byte_write_overload_v<Args...>
	;
}