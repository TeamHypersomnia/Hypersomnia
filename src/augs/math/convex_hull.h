#pragma once
#include <vector>
#include "augs/templates/algorithm_templates.h"

namespace augs {
	template <class C>
	auto convex_hull(C& P) {
		auto n = P.size();
		auto k = std::size_t(0);

		if (n <= 3) {
			return P;
		}

		using R = C;

		R H;
		H.resize(n * 2);
	   
		auto pred = [](const auto& a, const auto& b) {
			return a.x < b.x || (a.x == b.x && a.y < b.y);
		};
				
		auto cross = [](const auto& O, const auto& A, const auto& B) {
			return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
		};

		sort_range(P, pred);

		for (std::size_t i = 0; i < n; ++i) {
			while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) {
				k--;
			}

			H[k++] = P[i];
		}

		for (std::size_t i = n-1, t = k+1; i > 0; --i) {
			while (k >= t && cross(H[k-2], H[k-1], P[i-1]) <= 0) {
				k--;
			}

			H[k++] = P[i-1];
		}

		H.resize(k-1);
		return H;
	}
}

