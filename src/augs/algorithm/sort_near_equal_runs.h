#pragma once
#include <algorithm>
#include <cstddef>
#include "augs/math/repro_math.h"

namespace augs {
	/*
		For a container already sorted by a key: re-sorts with `less` every run of neighbours
		whose keys differ by at most eps.

		This is how "equal within an epsilon, then by something else" gets sorted. Such an order can't be
		the comparator of std::sort: equality within an epsilon isn't transitive, and std::sort's result then
		depends on the standard library - a desync between platforms. A run here is a chain of neighbours,
		so a ~ b ~ c group together even when a and c lie farther apart.

		`less` must be a full order, so that equal elements are identical and their order never matters.
	*/

	template <class Container, class GetKey, class T, class Less>
	void sort_near_equal_runs(Container& sorted, GetKey&& get_key, const T eps, Less&& less) {
		const auto n = sorted.size();
		auto run_begin = static_cast<std::size_t>(0);

		for (std::size_t i = 1; i <= n; ++i) {
			const bool run_continues = i < n && repro::fabs(get_key(sorted[i]) - get_key(sorted[i - 1])) <= eps;

			if (run_continues) {
				continue;
			}

			if (i - run_begin > 1) {
				std::sort(sorted.begin() + run_begin, sorted.begin() + i, less);
			}

			run_begin = i;
		}
	}
}
