#include "ingredients.h"
#include "entity_system/entity.h"
#include "entity_system/world.h"

#include "game/components/behaviour_tree_component.h"
#include "game/components/pathfinding_component.h"
#include "game/components/visibility_component.h"

#include "game/globals/filters.h"
#include "game/settings.h"

namespace ingredients {
	void standard_pathfinding_capability(augs::entity_id e) {
		auto& pathfinding = *e += components::pathfinding();
		auto& visibility = *e += components::visibility();

		pathfinding.mark_touched_as_discovered = true;
		pathfinding.force_persistent_navpoints = true;
		pathfinding.enable_backtracking = true;
		pathfinding.target_offset = 100;
		pathfinding.rotate_navpoints = 10;
		pathfinding.distance_navpoint_hit = 2;
		pathfinding.favor_velocity_parallellness = true;

		components::visibility::layer layer;
		layer.square_side = 5000;
		layer.color.set(0, 255, 255, 120);
		layer.filter = filters::pathfinding_query();

		visibility.add_layer(components::visibility::DYNAMIC_PATHFINDING, layer);
	}

	void enemy_intelligence(augs::entity_id e) {
		auto& behaviour_tree = *e += components::behaviour_tree();
		auto& trees = behaviour_tree.concurrent_trees;

		trees.resize(4);
		
		trees[0].state.user_input = e;
		trees[0].tree_id = assets::behaviour_tree_id::HANDS_ACTOR;
		trees[1].state.user_input = e;
		trees[1].tree_id = assets::behaviour_tree_id::INVENTORY_ACTOR;
		trees[2].state.user_input = e;
		trees[2].tree_id = assets::behaviour_tree_id::ITEM_PICKER;
		trees[3].state.user_input = e;
		trees[3].tree_id = assets::behaviour_tree_id::SOLDIER_MOVEMENT;
	}
}