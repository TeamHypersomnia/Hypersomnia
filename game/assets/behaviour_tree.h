#pragma once

namespace resources {
	class behaviour_tree;
}

namespace assets {
	enum behaviour_tree_id {
		SOLDIER_MOVEMENT,
		ITEM_PICKER,
		HANDS_ACTOR,
		INVENTORY_ACTOR
	};
}

resources::behaviour_tree& operator*(const assets::behaviour_tree_id& id);