#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/readwrite/byte_readwrite_traits.h"
#include "augs/readwrite/pointer_to_buffer.h"

namespace augs {
	template <class B, class S>
	auto make_ptr_write_stream(B* const bytes, const S n) {
		static_assert(std::is_integral_v<S>);

		const auto buf = augs::pointer_to_buffer { 
			reinterpret_cast<std::byte*>(bytes), 
			static_cast<std::size_t>(n) 
		};

		auto stream = augs::ptr_memory_stream(buf);
		stream.set_write_pos(0);
		return stream;
	}

	template <class S>
	auto make_ptr_write_stream(const S& source) {
		return make_ptr_write_stream(source.data(), source.size());
	}

	template <class B, class S>
	auto make_ptr_read_stream(const B* const bytes, const S n) {
		static_assert(std::is_integral_v<S>);

		const auto buf = augs::cpointer_to_buffer { 
			reinterpret_cast<const std::byte*>(bytes), 
			static_cast<std::size_t>(n) 
		};

		return augs::cptr_memory_stream(buf);
	}

	template <class S>
	augs::cptr_memory_stream make_ptr_read_stream(const S& source) {
		return make_ptr_read_stream(source.data(), source.size());
	}

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

	template <class T, class B, class S>
	auto from_bytes(const B* buffer, const S num_bytes) {
		auto stream = augs::make_ptr_read_stream(buffer, num_bytes);
		return read_bytes<T>(stream);
	}

	void from_bytes(const std::vector<std::byte>& bytes, trivial_type_marker& object);
}
