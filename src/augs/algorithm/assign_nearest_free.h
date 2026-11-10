#pragma once
#include <cstddef>
#include <optional>
#include <vector>

namespace augs {
	template <class Pos>
	std::vector<std::optional<std::size_t>> assign_each_to_nearest_free(
		const std::vector<Pos>& froms,
		const std::vector<Pos>& targets
	) {
		auto assigned = std::vector<std::optional<std::size_t>>(froms.size());
		auto taken = std::vector<bool>(targets.size(), false);

		for (std::size_t from_index = 0; from_index < froms.size(); ++from_index) {
			const auto& from = froms[from_index];
			auto& nearest = assigned[from_index];

			for (std::size_t target_index = 0; target_index < targets.size(); ++target_index) {
				if (taken[target_index]) {
					continue;
				}

				const auto distance_sq = (targets[target_index] - from).length_sq();

				if (!nearest.has_value() || distance_sq < (targets[*nearest] - from).length_sq()) {
					nearest = target_index;
				}
			}

			if (nearest.has_value()) {
				taken[*nearest] = true;
			}
		}

		return assigned;
	}
}
