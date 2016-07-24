#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct transform;
	struct sprite;
	struct polygon;
}

class convex_partitioned_shape {
public:
	float radius = 0.f;

	enum {
		RECT,
		POLYGON,
		CIRCLE
	} type = RECT;

	vec2 rect_size;

	std::vector<std::vector<vec2>> convex_polys;
	std::vector<vec2> debug_original;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			CEREAL_NVP(radius),

			CEREAL_NVP(type),

			CEREAL_NVP(rect_size),

			CEREAL_NVP(convex_polys),
			CEREAL_NVP(debug_original)
		);
	}

	void offset_vertices(components::transform);
	void mult_vertices(vec2);

	void add_convex_polygon(const std::vector<vec2>&);
	void add_concave_polygon(const std::vector<vec2>&);

	void from_renderable(const_entity_handle);
	void from_sprite(const components::sprite&, bool polygonize);
	void from_polygon(const components::polygon&);
};