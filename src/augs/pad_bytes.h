#pragma once
#include <array>
#include <cstddef>

template <std::size_t count>
struct pad_bytes {
	// GEN INTROSPECTOR struct pad_bytes std::size_t count
	std::array<std::byte, count> pad;
	// END GEN INTROSPECTOR

	pad_bytes() {
		for (auto& p : pad) {
			p = static_cast<std::byte>(0);
		}
	}

	bool operator==(const pad_bytes<count>& b) const {
		return pad == b.pad;
	}

	bool operator!=(const pad_bytes<count>& b) const {
		return pad != b.pad;
	}
};

template <class T>
struct is_padding_field : std::false_type {};

template <std::size_t I>
struct is_padding_field<pad_bytes<I>> : std::true_type {};

template <class T>
constexpr bool is_padding_field_v = is_padding_field<T>::value;