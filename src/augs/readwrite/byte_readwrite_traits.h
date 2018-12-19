#pragma once
#include "augs/templates/remove_cref.h"
#include "augs/readwrite/byte_readwrite_overload_traits.h"

namespace augs {
	template <class T, class = void>
	struct has_byte_write : std::false_type {};

	template <class T>
	struct has_byte_write<
		T, 
		decltype(
			std::declval<T&>().write((const char*)nullptr, 0), 
			void()
		)
	> : std::true_type {};

	template <class T>
	struct has_byte_write<
		T, 
		decltype(
			std::declval<T&>().write((const std::byte*)nullptr, 0), 
			void()
		)
	> : std::true_type {};

	template <class T, class = void>
	struct has_byte_read : std::false_type {};

	template <class T>
	struct has_byte_read<
		T, 
		decltype(
			std::declval<T&>().read((char*)nullptr, 0), 
			void()
		)
	> : std::true_type {};

	template <class T>
	struct has_byte_read<
		T, 
		decltype(
			std::declval<T&>().read((std::byte*)nullptr, 0), 
			void()
		)
	> : std::true_type {};

	template <class Archive>
	constexpr bool is_byte_stream_v = 
		has_byte_read<remove_cref<Archive>>::value 
		|| has_byte_write<remove_cref<Archive>>::value
	;

	template <class Archive, class Serialized>
	constexpr bool is_byte_readwrite_safe_v = is_byte_stream_v<Archive> && std::is_trivially_copyable_v<Serialized>;

	template <class Archive, class Serialized>
	constexpr bool is_byte_readwrite_appropriate_v = 
		is_byte_readwrite_safe_v<Archive, Serialized> 
		&& !has_byte_readwrite_overloads_v<Archive, Serialized>
	;

	template <class Archive, class Serialized>
	void verify_byte_readwrite_safety() {
		static_assert(std::is_trivially_copyable_v<Serialized>, "Attempt to serialize a non-trivially copyable type");
		static_assert(is_byte_stream_v<Archive>, "Byte serialization of trivial structs allowed only on native binary archives");
	}
}