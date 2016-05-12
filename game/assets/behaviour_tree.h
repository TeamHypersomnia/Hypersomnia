#pragma once

namespace resources {
	class behaviour_tree;
}

namespace assets {
	enum behaviour_tree_id {
		SOLDIER_TREE,
	};
}

resources::behaviour_tree& operator*(const assets::behaviour_tree_id& id);