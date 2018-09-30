#include "augs/misc/convex_partitioned_shape.h"

template <
	class T,
	std::size_t V,
	std::size_t P
>
template <class F>
void basic_convex_partitioned_shape<T, V, P>::for_each_convex(F&& callback) const {
	if (take_vertices_one_after_another()) {
		callback(original_poly);
		return;
	}

	using C = basic_convex_partitioned_shape<T, V, P>;

	typename C::original_poly_type convex;

	const auto& partition = convex_partition;

	std::size_t last_i = 0;

	for (std::size_t i = 0; i < partition.size(); ++i) {
		if ((i - last_i) > 0 && partition[i] == partition[last_i]) {
			callback(convex);
			last_i = i + 1;
			convex.clear();
			continue;
		}

		convex.push_back(original_poly[partition[i]]);
	}
}
