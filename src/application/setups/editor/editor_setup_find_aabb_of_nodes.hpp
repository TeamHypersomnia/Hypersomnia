#pragma once
#include "view/rendering_scripts/find_aabb_of.h"

template <class F>
std::optional<ltrb> editor_setup::find_aabb_of_typed_nodes(F&& for_each_typed_node) const {
	ltrb total;

	for_each_typed_node(
		[&](const auto id) {
			if (const auto node = find_node(id)) {
				// TODO: In case of prefabs, this should consider *all* entity ids, not just the scene_entity_id.

				if (const auto handle = scene.world[node->scene_entity_id]) {
					if (const auto aabb = find_aabb_of_entity(handle)) {
						total.contain(*aabb);
					}
				}
			}
		}
	);

	if (total.good()) {
		return total;
	}

	return std::nullopt;
}

template <class F>
std::optional<ltrb> editor_setup::find_aabb_of_nodes(F&& for_each_node) const {
	ltrb total;

	for_each_node(
		[&](const auto id) {
			on_node(id, [this, &total](const auto& typed_node, const auto typed_id) {
				(void)typed_id;

				LOG_NVPS(typed_node.get_display_name());
				if (const auto handle = scene.world[typed_node.scene_entity_id]) {
					LOG_NVPS(handle);
					if (const auto aabb = find_aabb_of_entity(handle)) {
						LOG_NVPS(*aabb);
						total.contain(*aabb);
					}
				}
			});
		}
	);

	LOG_NVPS(total);
	if (total.good()) {
		return total;
	}

	return std::nullopt;
}
