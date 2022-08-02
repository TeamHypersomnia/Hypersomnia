#pragma once
#include <string>

namespace augs {
	template <class F>
	std::string first_free_string(
		const std::string& pattern,
		const std::string& index_pattern,
		F&& is_free
	) {
		for (std::size_t i = 0;; ++i) {
			const auto index_str = typesafe_sprintf(index_pattern, i);
			//const auto candidate = i ? typesafe_sprintf(pattern, index_str) : typesafe_sprintf(pattern, "");
			const auto candidate = i ? (pattern + index_str) : pattern;

			if (is_free(candidate)) {
				return candidate;
			}
		}
	}
}
