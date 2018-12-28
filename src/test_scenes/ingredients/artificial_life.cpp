#include "test_scenes/ingredients/ingredients.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/components/behaviour_tree_component.h"
#include "game/components/pathfinding_component.h"
#include "game/messages/visibility_information.h"

#include "game/enums/filters.h"
#include "game/cosmos/entity_handle.h"

namespace ingredients {
	void add_standard_pathfinding_capability(entity_handle) {
		//pathfinding.mark_touched_as_discovered = true;
		//pathfinding.force_persistent_navpoints = true;
		//pathfinding.enable_backtracking = false;
		//pathfinding.target_offset = 100;
		//pathfinding.rotate_navpoints = 10;
		//pathfinding.distance_navpoint_hit = 2;
		//pathfinding.favor_velocity_parallellness = true;
		//pathfinding.filter = predefined_queries::pathfinding();
		//
		//auto& layer = visibility.full_visibility_layers[invariants::visibility::DYNAMIC_PATHFINDING];
		//layer.queried_rect = vec2::square(1000);
		//layer.color.set(0, 255, 255, 120);
		//layer.filter = predefined_queries::pathfinding();
	}

	void add_soldier_intelligence(entity_handle) {
		// auto& los = e.get<invariants::visibility>().line_of_sight_layers[invariants::visibility::LINE_OF_SIGHT];
		// los.test_sentiences = true;
		// los.test_attitudes = true;
		// los.test_items = true;
		// los.test_dangers = true;
		// los.maximum_distance = 1000.0;
		// los.obstruction_filter = predefined_queries::line_of_sight();
		// los.candidate_filter = filters[predefined_filter_type::LINE_OF_SIGHT_CANDIDATES];
		// 
		// auto& behaviour_tree = e += invariants::behaviour_tree();
		// auto& trees = behaviour_tree.concurrent_trees;
		// 
		// trees.resize(3);
		// 
		// trees[0].tree_id = assets::behaviour_tree_id::HOSTILE_TARGET_PRIORITIZATION;
		// trees[1].tree_id = assets::behaviour_tree_id::SOLDIER_MOVEMENT;
		// trees[2].tree_id = assets::behaviour_tree_id::HANDS_ACTOR;
		// 
		// //trees[1].tree_id = assets::behaviour_tree_id::INVENTORY_ACTOR;
		// //trees[2].tree_id = assets::behaviour_tree_id::ITEM_PICKER;
	}
}