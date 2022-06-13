#pragma once
#include "augs/string/typesafe_sprintf.h"

namespace augs {
	template <class F = true_returner>
	path_type find_first(const free_path_type type, const path_type& path_template, F&& allow_candidate = true_returner()) {
		for (std::size_t candidate = 0;; ++candidate) {
			const auto candidate_path = candidate ? 
				typesafe_sprintf(path_template.string(), candidate)
				: typesafe_sprintf(path_template.string(), "")
			;

			auto result = [&]() {
				if (!augs::exists(candidate_path)) {
					return candidate_path;
				}

				if (type == free_path_type::EMPTY) {
					if (augs::is_empty(candidate_path)) {
						return candidate_path;
					}
				}

				return std::string();
			}();

			if (result.empty()) {
				continue;
			}

			if (allow_candidate(result)) {
				return result;
			}
		}
	}

	template <class... Args>
	decltype(auto) first_empty_path(Args&&... args) {
		return find_first(free_path_type::EMPTY, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) first_free_path(Args&&... args) {
		return find_first(free_path_type::NON_EXISTING, std::forward<Args>(args)...);
	}
}
