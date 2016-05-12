#pragma once
#include "all.h"
#include "game/resources/behaviour_tree.h"
#include "game/resources/manager.h"
#include "graphics/shader.h"

namespace resource_setups {
	void load_standard_behaviour_trees() {
		auto& soldier_tree = resource_manager.create(assets::behaviour_tree_id::SOLDIER_TREE);



		soldier_tree.root;
		soldier_tree.build_tree();
	}
}