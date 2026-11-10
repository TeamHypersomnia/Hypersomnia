#pragma once
#include <unordered_set>
#include "application/setups/editor/project/editor_project.h"

inline std::unordered_set<editor_node_id> make_node_set_of_layers_hiding_icons(const editor_project& project) {
	std::unordered_set<editor_node_id> result;

	for (const auto layer_id : project.layers.order) {
		if (const auto layer = project.find_layer(layer_id)) {
			if (!layer->editable.show_icons) {
				result.insert(layer->hierarchy.nodes.begin(), layer->hierarchy.nodes.end());
			}
		}
	}

	return result;
}
