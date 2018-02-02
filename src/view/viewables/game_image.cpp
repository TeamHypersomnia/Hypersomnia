#include "view/viewables/game_image.h"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"
#include "game/common_state/entity_flavours.h"
#include "game/components/shape_polygon_component.h"
#include "view/viewables/regeneration/game_image_loadables.h"

game_image_cache::game_image_cache(
	const game_image_loadables& loadables,
	const game_image_meta& meta
) { 
	original_image_size = loadables.read_source_image_size();
	partitioned_shape.make_box(vec2(original_image_size));
}

loaded_game_image_caches::loaded_game_image_caches(
	const game_image_loadables_map& loadables,
	const game_image_metas_map& metas
) { 
	for (const auto& l : loadables) {
		game_image_cache ch(l.second, metas.at(l.first));

		emplace(l.first, ch);
	}
}

void add_shape_invariant_from_renderable(
	entity_flavour& into,
	const loaded_game_image_caches& caches
) {
	static_assert(into.has<invariants::shape_polygon>());

	if (const auto sprite = into.find<invariants::sprite>()) {
		const auto image_size = caches.at(sprite->tex).get_size();
		vec2 scale = sprite->get_size() / image_size;

		invariants::shape_polygon shape_polygon_def;

		shape_polygon_def.shape = caches.at(sprite->tex).partitioned_shape;
		shape_polygon_def.shape.scale(scale);

		into.set(shape_polygon_def);
	}

	if (const auto polygon = into.find<invariants::polygon>()) {
		std::vector<vec2> input;

		input.reserve(polygon->vertices.size());

		for (const auto& v : polygon->vertices) {
			input.push_back(v.pos);
		}

		invariants::shape_polygon shape_polygon_def;
		shape_polygon_def.shape.add_concave_polygon(input);

		into.set(shape_polygon_def);
	}
}
