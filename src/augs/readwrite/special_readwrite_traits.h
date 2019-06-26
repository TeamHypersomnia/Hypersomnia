#pragma once
#include <type_traits>
#include "augs/templates/type_list.h"

#define READWRITE_SPECIAL_TRAITS_INCLUDED 1

namespace augs {
	template <class ArgsList, class = void>
	struct has_special_read : std::false_type 
	{};

	template <class Archive, class Serialized>
	struct has_special_read <
		type_list<Archive, Serialized>,
		decltype(std::declval<Archive&>().special_read(std::declval<Serialized&>()), void())
	> : std::true_type 
	{};

	template <class ArgsList, class = void>
	struct has_special_write : std::false_type 
	{};

	template <class Archive, class Serialized>
	struct has_special_write <
		type_list<Archive, Serialized>,
		decltype(std::declval<Archive&>().special_write(std::declval<const Serialized&>()), void())
	> : std::true_type 
	{};

	template <class... Args>
	constexpr bool has_special_read_v = has_special_read<type_list<Args...>>::value;

	template <class... Args>
	constexpr bool has_special_write_v = has_special_write<type_list<Args...>>::value;

	template <class... Args>
	constexpr bool has_special_readwrites_v = 
		has_special_read_v<Args...> && has_special_write_v<Args...>
	;
}