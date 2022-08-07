#pragma once
#include "application/setups/editor/nodes/editor_node_id.h"

struct editor_prefab_node;
struct editor_prefab_resource {
	using node_type = editor_prefab_node;

	// GEN INTROSPECTOR struct editor_prefab_resource
	std::vector<editor_node_id> subnodes;
	// END GEN INTROSPECTOR

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Prefab";
	}
};

