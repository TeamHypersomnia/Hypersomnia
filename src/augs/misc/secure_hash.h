#pragma once
#include <vector>
#include <cstddef>
#include <string>

namespace augs {
	std::string secure_hash(const std::byte* bytes, std::size_t n);

	inline std::string secure_hash(const std::vector<std::byte>& bytes) {
		return secure_hash(bytes.data(), bytes.size());
	}
}

