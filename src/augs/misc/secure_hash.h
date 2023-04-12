#pragma once
#include <vector>
#include <cstddef>
#include <string>

#include "augs/misc/constant_size_string.h"

namespace augs {
	using secure_hash_type = std::array<uint8_t, 32>;
	using hash_string_type = augs::constant_size_string<64>;

	hash_string_type to_hex_format(const secure_hash_type& hstr);

	secure_hash_type secure_hash(const std::byte* bytes, std::size_t n);

	inline secure_hash_type secure_hash(const std::string& bytes) {
		return secure_hash(
			reinterpret_cast<const std::byte*>(bytes.data()),
			bytes.size()
		);
	}

	inline secure_hash_type secure_hash(const std::vector<std::byte>& bytes) {
		return secure_hash(bytes.data(), bytes.size());
	}
}

