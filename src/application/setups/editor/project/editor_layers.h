#pragma once
#include <string>
#include <unordered_map>
#include "application/setups/editor/nodes/editor_node_id.h"
#include "augs/misc/pool/pool.h"

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
	bool gui_open = true;

	editor_layer_hierarchy hierarchy;
	// END GEN INTROSPECTOR

	bool passed_filter = false;
};

using editor_layer_id = augs::pooled_object_id<unsigned short, editor_layer>;
using editor_layer_pool = augs::pool<editor_layer, make_vector, unsigned short, type_list<>, editor_layer>;

struct editor_layers {
	static constexpr bool json_ignore = true;

	std::vector<editor_layer_id> order;
	editor_layer_pool pool;

	auto make_name_to_layer_map() {
		std::unordered_map<std::string, editor_layer*> out;

		for (auto& layer : pool) {
			out[layer.name] = std::addressof(layer);
		}

		return out;
	}
};
