#include "dynamic_tree_node_component.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"

#include "game/transcendental/cosmos.h"

namespace components {
	dynamic_tree_node dynamic_tree_node::get_default(const_entity_handle e) {
		dynamic_tree_node result;

		auto* sprite = e.find<components::sprite>();
		auto* polygon = e.find<components::polygon>();
		//auto* tile_layer_instance = e.find<components::tile_layer_instance>();
		auto* particles_existence = e.find<components::particles_existence>();

		auto transform = e.logic_transform();

		if (sprite) result.aabb = sprite->get_aabb(transform);
		if (polygon) result.aabb = polygon->get_aabb(transform);
		//if (tile_layer_instance) result.aabb = tile_layer_instance->get_aabb(transform);

		if (particles_existence) result.always_visible = true;

		return result;
	}
}