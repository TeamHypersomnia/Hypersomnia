#pragma once
#include <string>

namespace augs {
	/*
		Compares secrets (e.g. a state the browser brings back) without returning at the first difference,
		so the time taken tells nothing about how much of a guess was right.
	*/

	inline bool constant_time_equal(const std::string& a, const std::string& b) {
		if (a.size() != b.size()) {
			return false;
		}

		unsigned char difference = 0;

		for (std::size_t i = 0; i < a.size(); ++i) {
			difference |= static_cast<unsigned char>(a[i] ^ b[i]);
		}

		return difference == 0;
	}
}
