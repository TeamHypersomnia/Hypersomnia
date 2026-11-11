#pragma once
#include "game/cosmos/cosmos.h"
#include "game/components/movement_path_component.h"
#include "application/setups/editor/nodes/editor_sprite_node.h"

template <class P>
void set_orbit_center_at_orbited_node(
	const editor_sprite_node& orbiting_node,
	const P& project,
	cosmos& cosm
) {
	const auto orbited_node = project.find_node(orbiting_node.editable.orbit_around);

	if (orbited_node == nullptr) {
		return;
	}

	if (const auto handle = cosm[orbiting_node.scene_entity_id]) {
		handle.dispatch([&](const auto& typed_handle) {
			if (const auto movement_path = typed_handle.template find<components::movement_path>()) {
				movement_path->orbit_center.emplace(orbited_node->editable.pos);
			}
		});
	}
}
