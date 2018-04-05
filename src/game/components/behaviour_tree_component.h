#pragma once
#include "game/assets/behaviour_tree.h"
#include "game/assets/ids/behaviour_tree_id.h"

#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

struct behaviour_tree_instance {
	// GEN INTROSPECTOR struct behaviour_tree_instance
	state_of_behaviour_tree_instance state;
	assets::behaviour_tree_id tree_id = assets::behaviour_tree_id::INVALID;
	// END GEN INTROSPECTOR
};

namespace components {
	struct behaviour_tree {
		// GEN INTROSPECTOR struct components::behaviour_tree
		augs::constant_size_vector<behaviour_tree_instance, CONCURRENT_TREES_COUNT> concurrent_trees = {};
		// END GEN INTROSPECTOR
	};
}

