#pragma once
#include "game/cosmos/per_entity_type.h"
#include "application/setups/editor/nodes/editor_node_id.h"

using scene_entity_to_node_map = per_entity_type_array<std::vector<editor_node_id>>;

inline editor_node_id entity_to_node_id(const scene_entity_to_node_map& scene_entity_to_node, const entity_id& id) {
	if (!id.is_set()) {
		return {};
	}

	const auto& mapping = scene_entity_to_node[id.type_id.get_index()];
	const auto node_index = id.raw.indirection_index;

	if (node_index < mapping.size()) {
		return mapping[node_index];
	}

	return {};
}
