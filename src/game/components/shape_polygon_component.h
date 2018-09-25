#pragma once
#include "augs/pad_bytes.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/convex_partitioned_shape.h"
#include "game/container_sizes.h"

using logic_convex_poly = basic_convex_partitioned_shape<
	real32,
	POLY_VERTEX_COUNT,
	POLY_PARTITION_INDEX_COUNT
>;

struct b2Fixture_index_in_component;

struct convex_poly_destruction_scar {
	// GEN INTROSPECTOR struct convex_poly_destruction_scar
	vec2 first_impact;
	vec2 depth_point;
	// END GEN INTROSPECTOR
};

struct convex_poly_destruction_data {
	// GEN INTROSPECTOR struct convex_poly_destruction_data
	augs::constant_size_vector<convex_poly_destruction_scar, DESTRUCTION_SCARS_COUNT> scars = {};
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct shape_polygon {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::shape_polygon
		logic_convex_poly shape;
		// END GEN INTROSPECTOR
	};
}