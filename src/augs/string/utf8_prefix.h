#pragma once
#include <cstddef>
#include <string>

namespace augs {
	inline std::string utf8_prefix_never_cut_inside_a_character(const std::string& text, const std::size_t max_bytes) {
		if (text.size() <= max_bytes) {
			return text;
		}

		auto prefix_bytes = max_bytes;

		while (prefix_bytes > 0 && (static_cast<unsigned char>(text[prefix_bytes]) & 0xC0) == 0x80) {
			--prefix_bytes;
		}

		return text.substr(0, prefix_bytes);
	}
}
