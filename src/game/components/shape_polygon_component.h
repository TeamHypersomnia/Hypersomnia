#pragma once
#include "augs/pad_bytes.h"

#include "augs/misc/constant_size_vector.h"
#include "augs/misc/enum/enum_array.h"
#include "augs/misc/convex_partitioned_shape.h"

#include "game/assets/all_logical_assets_declarations.h"
#include "game/transcendental/component_synchronizer.h"

#include "game/components/all_inferred_state_component.h"

struct b2Fixture_index_in_component;

struct convex_poly_destruction_scar {
	// GEN INTROSPECTOR struct convex_poly_destruction_scar
	vec2 first_impact;
	vec2 depth_point;
	// END GEN INTROSPECTOR
};

struct convex_poly_destruction_data {
	// GEN INTROSPECTOR struct convex_poly_destruction_data
	augs::constant_size_vector<convex_poly_destruction_scar, DESTRUCTION_SCARS_COUNT> scars;
	// END GEN INTROSPECTOR
};

namespace definitions {
	struct shape_polygon {
		// GEN INTROSPECTOR struct definitions::shape_polygon
		convex_partitioned_shape shape;
		// END GEN INTROSPECTOR
	};
}