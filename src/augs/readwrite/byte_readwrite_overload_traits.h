#pragma once
#include <type_traits>
#include "augs/readwrite/memory_stream_declaration.h"

#define READWRITE_OVERLOAD_TRAITS_INCLUDED 1

namespace augs {
	class byte_counter_stream;
	
	template <class Archive, class Serialized, class = void>
	struct has_byte_read_overload : std::false_type 
	{};

	template <class Archive, class Serialized>
	struct has_byte_read_overload <
		Archive,
		Serialized,
		decltype(
			read_object_bytes(
				std::declval<Archive&>(),
				std::declval<Serialized&>()
			),
			void()
		)
	> : std::true_type 
	{};

	template <class Archive, class Serialized, class = void>
	struct has_byte_write_overload : std::false_type 
	{};

	template <class Archive, class Serialized>
	struct has_byte_write_overload <
		Archive,
		Serialized,
		decltype(
			write_object_bytes(
				std::declval<Archive&>(),
				std::declval<const Serialized&>()
			),
			void()
		)
	> : std::true_type 
	{};


	template <class Archive, class Serialized>
	constexpr bool has_byte_read_overload_v = has_byte_read_overload<Archive, Serialized>::value;

	template <class Archive, class Serialized>
	constexpr bool has_byte_write_overload_v = has_byte_write_overload<Archive, Serialized>::value;

	template <class Archive, class Serialized>
	constexpr bool has_byte_readwrite_overloads_v = 
		has_byte_read_overload_v<Archive, Serialized> && has_byte_write_overload_v<Archive, Serialized>
	;
}