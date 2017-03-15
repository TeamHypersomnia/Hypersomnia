#include "shape_variant.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void shape_variant::from_renderable(const const_entity_handle handle) {
	if (handle.has<components::sprite>()) {
		convex_partitioned_shape coll;
		coll.from_sprite(handle.get<components::sprite>(), true);
		set(coll);
	}
	if (handle.has<components::polygon>()) {
		convex_partitioned_shape coll;

		std::vector<vec2> input;

		const auto& poly = handle.get<components::polygon>();

		input.reserve(poly.vertices.size());

		for (const auto& v : poly.vertices) {
			input.push_back(v.pos);
		}

		coll.add_concave_polygon(input);

		set(coll);
	}
}