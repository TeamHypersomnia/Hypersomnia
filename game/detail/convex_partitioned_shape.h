#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/build_settings.h"
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

namespace components {
	struct transform;
	struct sprite;
	struct polygon;
}

class convex_partitioned_shape {
public:
	float radius = 0.f;

	enum class shape_type {
		POLYGON,
		CIRCLE
	} type = shape_type::POLYGON;

	typedef augs::constant_size_vector<vec2, CONVEX_POLY_VERTEX_COUNT> convex_poly;
	augs::constant_size_vector<convex_poly, CONVEX_POLYS_COUNT> convex_polys;

#if ENABLE_POLYGONIZATION
	std::vector<vec2> debug_original;
#endif

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(radius),

			CEREAL_NVP(type),

			CEREAL_NVP(convex_polys)
		);
	}

	void offset_vertices(components::transform);
	void mult_vertices(vec2);

	void add_convex_polygon(const convex_poly&);
	void add_concave_polygon(const std::vector<vec2>&);

	void from_renderable(const_entity_handle);
	void from_sprite(const components::sprite&, bool polygonize);
	void from_polygon(const components::polygon&);
};