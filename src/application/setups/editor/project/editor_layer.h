#pragma once
#include <string>
#include "application/setups/editor/nodes/editor_node_id.h"

struct editor_layer_hierarchy {
	static constexpr bool json_ignore = true;

	// GEN INTROSPECTOR struct editor_layer_hierarchy
	std::vector<editor_node_id> nodes;
	// END GEN INTROSPECTOR
};

struct editor_layer {
	// GEN INTROSPECTOR struct editor_layer
	std::string name;
	bool visible = true;

	editor_layer_hierarchy hierarchy;
	// END GEN INTROSPECTOR
};
