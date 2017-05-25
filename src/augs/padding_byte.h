#pragma once
#include <array>

struct padding_byte {
	// GEN INTROSPECTOR struct padding_byte
	char byte = static_cast<char>(0);
	// END GEN INTROSPECTOR
};

template <std::size_t count>
struct pad_bytes {
	// GEN INTROSPECTOR struct pad_bytes std::size_t count
	std::array<padding_byte, count> pad;
	// END GEN INTROSPECTOR
};