#pragma once
#include <vector>
#include "augs/templates.h"

namespace augs {
	std::vector<size_t> run_length_encoding(const std::vector<bool>& bit_data);

	struct object_delta {
		std::vector<char> changed_bytes;
		std::vector<size_t> changed_offsets;
	};

	object_delta delta_encode(const char* base, const char* encoded, size_t length);

	template <class T>
	object_delta delta_encode(const T& base_object, const T& encoded_object) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to encode a non-trivially copyable type");

		return delta_encode(
			reinterpret_cast<const char*>(std::addressof(base_object)), 
			reinterpret_cast<const char*>(std::addressof(encoded_object)),
			sizeof(T));
	};

	void delta_decode(char* into, size_t length, const object_delta& delta);

	template <class T>
	void delta_decode(T& decoded, const object_delta& delta) {
		static_assert(is_memcpy_safe<T>::value, "Attempt to decode a non-trivially copyable type");

		delta_decode(reinterpret_cast<char*>(std::addressof(decoded)), sizeof(T), delta);
	};
}