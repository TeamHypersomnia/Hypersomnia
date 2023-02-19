#pragma once
#include <string>
#include <unordered_map>
#include "application/setups/editor/nodes/editor_node_id.h"
#include "augs/misc/pool/pool.h"
#include "application/setups/editor/project/editor_layer_id.h"
#include "augs/graphics/rgba.h"

struct editor_layer_hierarchy {
	// GEN INTROSPECTOR struct editor_layer_hierarchy
	std::vector<editor_node_id> nodes;
	// END GEN INTROSPECTOR
};

struct editor_layer_editable {
	// GEN INTROSPECTOR struct editor_layer_editable
	bool select_together = false;
	bool selectable_on_scene = true;
	float opacity = 1.0f;
	rgba tint = white;
	// END GEN INTROSPECTOR
};

struct editor_layer {
	// GEN INTROSPECTOR struct editor_layer
	std::string unique_name;
	editor_layer_editable editable;

	bool visible = true;
	bool is_open = true;

	editor_layer_hierarchy hierarchy;
	// END GEN INTROSPECTOR

	const auto& get_display_name() const {
		return unique_name;
	}

	bool empty() const {
		return hierarchy.nodes.empty();
	}

	bool passed_filter = false;
};

using editor_layer_pool_size_type = unsigned;
using editor_layer_pool = augs::pool<editor_layer, make_vector, editor_layer_pool_size_type, type_list<>, editor_layer>;

struct editor_layers {
	static constexpr bool json_ignore = true;

	std::vector<editor_layer_id> order;
	editor_layer_pool pool;

	auto make_name_to_layer_map() {
		std::unordered_map<std::string, editor_layer*> out;

		for (auto& layer : pool) {
			out[layer.unique_name] = std::addressof(layer);
		}

		return out;
	}
};
