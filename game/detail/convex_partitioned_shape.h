#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

namespace components {
	struct transform;
	struct sprite;
	struct polygon;
}

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

struct convex_poly {
	// GEN INTROSPECTOR struct convex_poly
	augs::constant_size_vector<vec2, CONVEX_POLY_VERTEX_COUNT> vertices;
	// END GEN INTROSPECTOR
};

struct convex_partitioned_shape {
	// GEN INTROSPECTOR struct convex_partitioned_shape
	augs::constant_size_vector<convex_poly, CONVEX_POLYS_COUNT> convex_polys;
	// END GEN INTROSPECTOR

	void offset_vertices(const components::transform);
	void scale(const vec2);

	void add_convex_polygon(const convex_poly&);
	void add_concave_polygon(const std::vector<vec2>&);
};