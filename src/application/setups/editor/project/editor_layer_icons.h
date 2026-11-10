#pragma once
#include <unordered_set>
#include "application/setups/editor/project/editor_project.h"

inline std::unordered_set<editor_node_id> make_node_set_of_layers_hiding_icons(const editor_project& project) {
	auto result = std::unordered_set<editor_node_id>();

	for (const auto layer_id : project.layers.order) {
		if (const auto layer = project.find_layer(layer_id)) {
			if (!layer->editable.show_icons) {
				result.insert(layer->hierarchy.nodes.begin(), layer->hierarchy.nodes.end());
			}
		}
	}

	return result;
}

inline std::unordered_set<entity_id> make_entity_set_of_layers_hiding_icons(const editor_project& project) {
	auto result = std::unordered_set<entity_id>();

	for (const auto node_id : ::make_node_set_of_layers_hiding_icons(project)) {
		project.on_node(node_id, [&](const auto& node, const auto) {
			if (node.scene_entity_id.is_set()) {
				result.insert(node.scene_entity_id);
			}
		});
	}

	return result;
}
