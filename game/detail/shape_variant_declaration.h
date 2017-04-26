#pragma once

namespace augs {
	template <class...>
	class trivial_variant;
}

struct convex_partitioned_shape;
struct circle_shape;

typedef augs::trivial_variant<
	convex_partitioned_shape
	//circle_shape
> shape_variant_base;
