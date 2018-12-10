#pragma once
#include <fstream>
#include <type_traits>

#include "augs/templates/introspect.h"
#include "augs/templates/folded_finders.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/templates/byte_type_for.h"

namespace augs {
	class byte_counter_stream;

	template <class T, class = void>
	struct has_byte_write : std::false_type {};

	template <class T>
	struct has_byte_write<
		T, 
		decltype(
			std::declval<T&>().write((const byte_type_for_t<T>*)nullptr, std::streamsize(0)), 
			void()
		)
	> : std::true_type {};

	template <class T, class = void>
	struct has_byte_read : std::false_type {};

	template <class T>
	struct has_byte_read<
		T, 
		decltype(
			std::declval<T&>().read((byte_type_for_t<T>*)nullptr, std::streamsize(0)), 
			void()
		)
	> : std::true_type {};

	template <class Archive>
	constexpr bool is_byte_stream_v = has_byte_read<remove_cref<Archive>>::value || has_byte_write<remove_cref<Archive>>::value;

	template <class Archive, class Serialized>
	constexpr bool is_byte_readwrite_safe_v = is_byte_stream_v<Archive> && std::is_trivially_copyable_v<Serialized>;

	template <class Archive, class Serialized>
	constexpr bool is_byte_readwrite_appropriate_v = is_byte_readwrite_safe_v<Archive, Serialized> && !has_byte_readwrite_overloads_v<Archive, Serialized>;

	template <class Archive, class Serialized>
	void verify_byte_readwrite_safety() {
		static_assert(std::is_trivially_copyable_v<Serialized>, "Attempt to serialize a non-trivially copyable type");
		static_assert(is_byte_stream_v<Archive>, "Byte serialization of trivial structs allowed only on native binary archives");
	}

	template <class Serialized>
	void verify_has_introspect(const Serialized&) {
		static_assert(has_introspect_v<Serialized>, "Attempt to serialize a type in a non-bytesafe context without i/o overloads and without introspectors!");
	}
}