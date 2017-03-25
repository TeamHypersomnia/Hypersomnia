#pragma once
#include <vector>
#include "augs/templates/memcpy_safety.h"

namespace augs {
	typedef unsigned short delta_offset_type;

	std::vector<delta_offset_type> run_length_encoding(const std::vector<bool>& bit_data);

	typedef char delta_unit;

	struct object_delta {
		std::vector<delta_unit> changed_bytes;
		std::vector<delta_offset_type> changed_offsets;
	};

	object_delta delta_encode(
		const delta_unit* const base, 
		const delta_unit* const encoded, 
		const size_t length_bytes
	);

	template <class T>
	object_delta delta_encode(const T& base_object, const T& encoded_object) {
		static_assert(is_memcpy_safe_v<T>, "Attempt to encode a type that is not trivially copyable");

		return delta_encode(
			reinterpret_cast<const delta_unit*>(std::addressof(base_object)), 
			reinterpret_cast<const delta_unit*>(std::addressof(encoded_object)),
			sizeof(T)
		);
	};

	void delta_decode(
		delta_unit* into, 
		const size_t length_bytes, 
		const object_delta& delta
	);

	template <class T>
	void delta_decode(T& decoded, const object_delta& delta) {
		static_assert(is_memcpy_safe_v<T>, "Attempt to decode a type that is not trivially copyable");

		delta_decode(reinterpret_cast<delta_unit*>(std::addressof(decoded)), sizeof(T), delta);
	};
}