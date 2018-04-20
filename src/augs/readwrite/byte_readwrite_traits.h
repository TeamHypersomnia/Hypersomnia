#pragma once
#include <fstream>
#include <type_traits>

#include "augs/templates/introspect.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/readwrite/memory_stream_declaration.h"

namespace augs {
	class byte_counter_stream;

	template <class Archive>
	constexpr bool is_byte_stream_v = is_derived_from_any_of_v<
		std::decay_t<Archive>,
		memory_stream,
		ref_memory_stream,
		cref_memory_stream,
		byte_counter_stream,
		std::ifstream,
		std::ofstream
	>;

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