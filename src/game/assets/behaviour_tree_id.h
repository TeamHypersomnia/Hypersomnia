#pragma once

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