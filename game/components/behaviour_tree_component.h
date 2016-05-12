#pragma once
#include <vector>
#include <unordered_map>

#include "entity_system/entity.h"
#include "misc/timer.h"

#include "game/resources/behaviour_tree.h"
#include "game/assets/behaviour_tree.h"

namespace components {
	struct behaviour_tree {
		resources::behaviour_tree::state_of_tree_instance state;
		assets::behaviour_tree_id tree_id;
	};
}

