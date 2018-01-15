#include "game/common_state/entity_types.h"

void entity_type::add_shape_definition_from_renderable(
	const all_logical_assets& metas
) {
	const auto& metas = step.get_logical_assets();

	if (const auto sprite = find<definitions::sprite>()) {
		const auto image_size = metas.at(sprite->tex).get_size();
		vec2 scale = sprite->get_size() / image_size;

		definitions::shape_polgon shape_polygon_def;

		shape_polygon_def.shape = metas.at(sprite->tex).shape;
		shape_polygon_def.shape.scale(scale);

		set(shape_polygon_def);
	}

	if (const auto polygon = find<definitions::polygon>()) {
		std::vector<vec2> input;

		input.reserve(polygon->vertices.size());

		for (const auto& v : polygon->vertices) {
			input.push_back(v.pos);
		}

		definitions::shape_polgon shape_polygon_def;
		shape_polygon_def.shape.add_concave_polygon(input);

		set(shape_polygon_def);
	}
}
