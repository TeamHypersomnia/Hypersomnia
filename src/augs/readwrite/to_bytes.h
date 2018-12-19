#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/readwrite/byte_readwrite_traits.h"

namespace augs {
	template <class B, class T>
	void assign_bytes(B& buffer, const T& object) {
		buffer.clear();
		auto s = basic_ref_memory_stream<B>(buffer);
		augs::write_bytes(s, object);
	}

	template <class B, class T>
	void to_bytes(B& buffer, const T& object) {
		auto s = basic_ref_memory_stream<B>(buffer);
		augs::write_bytes(s, object);
	}

	template <class B, class T>
	void from_bytes(const B& bytes, T& object) {
		auto s = basic_ref_memory_stream<const B>(bytes);
		augs::read_bytes(s, object);
	}

	template <class T>
	auto to_bytes(const T& object) {
		std::conditional_t<
			is_byte_readwrite_appropriate_v<memory_stream, T>,
			augs::constant_size_vector<std::byte, sizeof(T)>,
			std::vector<std::byte> 
		> s;

		to_bytes(s, object);
		return s;
	}

	struct trivial_type_marker {};

	template <class T, class B>
	auto from_bytes(const B& bytes) {
		static_assert(
			!std::is_same_v<T, trivial_type_marker>,
			"Use the other overload that takes destination as argument."
		);

		T object;
		from_bytes(bytes, object);
		return object;
	}

	void from_bytes(const std::vector<std::byte>& bytes, trivial_type_marker& object);
}
