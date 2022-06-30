#pragma once
#include "application/setups/editor/nodes/editor_node_id.h"

struct editor_prefab_resource {
	// GEN INTROSPECTOR struct editor_prefab_resource
	std::vector<editor_node_id> subnodes;
	// END GEN INTROSPECTOR

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};

