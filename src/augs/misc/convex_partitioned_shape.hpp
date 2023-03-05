#include "augs/misc/convex_partitioned_shape.h"

template <
	class T,
	std::size_t V,
	std::size_t P
>
template <class F>
void basic_convex_partitioned_shape<T, V, P>::for_each_convex(F&& callback) const {
	if (take_vertices_one_after_another()) {
		callback(source_polygon);
		return;
	}

	using C = basic_convex_partitioned_shape<T, V, P>;

	typename C::source_polygon_type convex;

	const auto& partition = convex_partition;

	std::size_t last_i = 0;

	for (std::size_t i = 0; i < partition.size(); ++i) {
		if ((i - last_i) > 0 && partition[i] == partition[last_i]) {
			callback(convex);
			last_i = i + 1;
			convex.clear();
			continue;
		}

		if (partition[i] < source_polygon.size()) {
			convex.push_back(source_polygon[partition[i]]);
		}
	}
}
