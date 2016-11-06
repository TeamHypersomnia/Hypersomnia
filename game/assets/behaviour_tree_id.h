#pragma once

namespace resources {
	class behaviour_tree;
}

namespace assets {
	enum class behaviour_tree_id {
		INVALID,

		SOLDIER_MOVEMENT,
		HOSTILE_TARGET_PRIORITIZATION,
		ITEM_PICKER,
		HANDS_ACTOR,
		INVENTORY_ACTOR,
		COUNT
	};
}

resources::behaviour_tree& operator*(const assets::behaviour_tree_id& id);