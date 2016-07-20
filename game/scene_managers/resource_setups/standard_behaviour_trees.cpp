#include "all.h"
#include "game/resources/behaviour_tree.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

#include "game/detail/ai/behaviours.h"

namespace resource_setups {
	void load_standard_behaviour_trees() {
		auto& soldier_movement = resource_manager.create(assets::behaviour_tree_id::SOLDIER_MOVEMENT);

		soldier_movement.root.mode = resources::behaviour_tree::node::type::SELECTOR;

		soldier_movement.root.branch(
			new behaviours::immediate_evasion,
			new behaviours::minimize_recoil_through_movement,
			new behaviours::navigate_to_last_seen_position_of_target,
			new behaviours::explore_in_search_for_last_seen_target
		);

		soldier_movement.build_tree();


		auto& hands_actor = resource_manager.create(assets::behaviour_tree_id::HANDS_ACTOR);

		hands_actor.root.branch(new behaviours::pull_trigger);

		hands_actor.root;
		hands_actor.build_tree();


		auto& item_picker = resource_manager.create(assets::behaviour_tree_id::ITEM_PICKER);


		//item_picker.root.branch(new behaviours::usable_item_marker);
		item_picker.build_tree();


		auto& inventory_actor = resource_manager.create(assets::behaviour_tree_id::INVENTORY_ACTOR);


		inventory_actor.root;
		inventory_actor.build_tree();

		auto& hostile_target_prioritization = resource_manager.create(assets::behaviour_tree_id::HOSTILE_TARGET_PRIORITIZATION);

		hostile_target_prioritization.root.branch(new behaviours::target_closest_enemy);
		hostile_target_prioritization.build_tree();
	}
}