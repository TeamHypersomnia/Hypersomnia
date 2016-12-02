#include "dynamic_tree_node_component.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/transform_component.h"
#include "game/components/wandering_pixels_component.h"

#include "game/transcendental/cosmos.h"

namespace components {
	dynamic_tree_node dynamic_tree_node::get_default(const_entity_handle e) {
		dynamic_tree_node result;

		const auto* const sprite = e.find<components::sprite>();
		const auto* const polygon = e.find<components::polygon>();
		const auto* const tile_layer_instance = e.find<components::tile_layer_instance>();
		const auto* const particles_existence = e.find<components::particles_existence>();
		const auto* const wandering_pixels = e.find<components::wandering_pixels>();

		if (sprite) {
			result.aabb = sprite->get_aabb(e.logic_transform());
		}
		else if (polygon) {
			result.aabb = polygon->get_aabb(e.logic_transform());
		}
		else if (tile_layer_instance) {
			result.aabb = tile_layer_instance->get_aabb(e.logic_transform());
		}
		else if (particles_existence) {
			result.always_visible = true;
		}
		else if (wandering_pixels) {
			result.aabb = wandering_pixels->reach;
		}

		return result;
	}
}