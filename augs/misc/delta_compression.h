#pragma once
#include <vector>
#include "augs/templates.h"

namespace augs {
	typedef unsigned short delta_offset_type;

	std::vector<delta_offset_type> run_length_encoding(const std::vector<bool>& bit_data);

	typedef char delta_unit;

	struct object_delta {
		std::vector<delta_unit> changed_bytes;
		std::vector<delta_offset_type> changed_offsets;
	};

	object_delta delta_encode(const delta_unit* base, const delta_unit* encoded, size_t length_bytes);

	template <class T>
	object_delta delta_encode(const T& base_object, const T& encoded_object) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to encode a non-trivially copyable type");

		return delta_encode(
			reinterpret_cast<const delta_unit*>(std::addressof(base_object)), 
			reinterpret_cast<const delta_unit*>(std::addressof(encoded_object)),
			sizeof(T));
	};

	void delta_decode(delta_unit* into, size_t length_bytes, const object_delta& delta);

	template <class T>
	void delta_decode(T& decoded, const object_delta& delta) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to decode a non-trivially copyable type");

		delta_decode(reinterpret_cast<delta_unit*>(std::addressof(decoded)), sizeof(T), delta);
	};
}