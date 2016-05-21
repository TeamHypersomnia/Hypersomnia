#pragma once
#include "all.h"
#include "game/resources/behaviour_tree.h"
#include "game/resources/manager.h"
#include "graphics/shader.h"

namespace resource_setups {
	void load_standard_behaviour_trees() {
		auto& soldier_movement = resource_manager.create(assets::behaviour_tree_id::SOLDIER_MOVEMENT);


		soldier_movement.root;
		soldier_movement.build_tree();


		auto& hands_actor = resource_manager.create(assets::behaviour_tree_id::HANDS_ACTOR);


		hands_actor.root;
		hands_actor.build_tree();


		auto& item_picker = resource_manager.create(assets::behaviour_tree_id::ITEM_PICKER);


		item_picker.root;
		item_picker.build_tree();


		auto& inventory_actor = resource_manager.create(assets::behaviour_tree_id::INVENTORY_ACTOR);


		inventory_actor.root;
		inventory_actor.build_tree();
	}
}