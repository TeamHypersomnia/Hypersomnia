#include "dynamic_tree_node_component.h"
#include "game/entity_handle.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_component.h"
#include "game/components/particle_group_component.h"
#include "game/components/transform_component.h"

namespace components {
	dynamic_tree_node dynamic_tree_node::get_default(const_entity_handle e) {
		dynamic_tree_node result;

		auto* sprite = e.find<components::sprite>();
		auto* polygon = e.find<components::polygon>();
		auto* tile_layer = e.find<components::tile_layer>();
		auto* particle_group = e.find<components::particle_group>();

		auto transform = e.get<components::transform>();

		if (sprite) result.aabb = sprite->get_aabb(transform);
		if (polygon) result.aabb = polygon->get_aabb(transform);
		if (tile_layer) result.aabb = tile_layer->get_aabb(transform);

		if (particle_group) result.always_visible = true;

		return result;
	}
}