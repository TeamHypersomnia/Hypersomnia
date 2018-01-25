#include "game/common_state/entity_types.h"
#include "game/assets/all_logical_assets.h"

void entity_type::add_shape_invariant_from_renderable(
	const all_logical_assets& metas
) {
	if (const auto sprite = find<invariants::sprite>()) {
		const auto image_size = metas.at(sprite->tex).get_size();
		vec2 scale = sprite->get_size() / image_size;

		invariants::shape_polygon shape_polygon_def;

		shape_polygon_def.shape = metas.at(sprite->tex).shape;
		shape_polygon_def.shape.scale(scale);

		set(shape_polygon_def);
	}

	if (const auto polygon = find<invariants::polygon>()) {
		std::vector<vec2> input;

		input.reserve(polygon->vertices.size());

		for (const auto& v : polygon->vertices) {
			input.push_back(v.pos);
		}

		invariants::shape_polygon shape_polygon_def;
		shape_polygon_def.shape.add_concave_polygon(input);

		set(shape_polygon_def);
	}
}
