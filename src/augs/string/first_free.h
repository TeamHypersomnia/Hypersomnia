#pragma once
#include <string>

namespace augs {
	template <class F>
	std::string first_free_string(
		const std::string& pattern,
		const std::string& index_pattern,
		F&& is_free,
		const bool add_first_number = false,
		const std::size_t starting_i = 0
	) {
		for (std::size_t i = starting_i;; ++i) {
			const auto index_str = typesafe_sprintf(index_pattern, i);
			//const auto candidate = i ? typesafe_sprintf(pattern, index_str) : typesafe_sprintf(pattern, "");
			const auto candidate = (add_first_number || i > starting_i) ? (pattern + index_str) : pattern;

			if (is_free(candidate)) {
				return candidate;
			}
		}
	}
}
