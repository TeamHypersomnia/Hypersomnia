#pragma once
#include "game/detail/visible_entities.h"
#include "game/detail/physics/shape_overlapping.hpp"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

template <class E>
bool bombsite_in_range(const E& fused_entity) {
	if (fused_entity.template get<components::hand_fuse>().defused()) {
		return false;
	}

	const auto& cosm = fused_entity.get_cosmos();
	const auto& fuse_def = fused_entity.template get<invariants::hand_fuse>();

	if (fuse_def.can_only_arm_at_bombsites) {
		const auto capability = fused_entity.get_owning_transfer_capability();
		const auto matched_faction = capability.alive() ? capability.get_official_faction() : faction_type::SPECTATOR;

		auto& entities = thread_local_visible_entities();

		entities.reacquire_all_and_sort({
			cosm,
			camera_cone::from_aabb(fused_entity),
			visible_entities_query::accuracy_type::PROXIMATE,
			render_layer_filter::whitelist(render_layer::AREA_MARKERS),
			{ { tree_of_npo_type::RENDERABLES } }
		});

		bool found = false;

		entities.for_each<render_layer::AREA_MARKERS>(cosm, [&](const auto& handle) {
			return handle.template dispatch_on_having_all_ret<invariants::box_marker>([&](const auto& typed_handle) {
				if constexpr(is_nullopt_v<decltype(typed_handle)>) {
					return callback_result::CONTINUE;
				}
				else {
					const auto& marker = typed_handle.template get<invariants::box_marker>();

					if (::is_bombsite(marker.type)) {
						if (matched_faction == typed_handle.get_official_faction()) {
							if (entity_overlaps_entity(typed_handle, fused_entity)) {
								found = true;
								return callback_result::ABORT;
							}
						}
					}

					return callback_result::CONTINUE;
				}
			});
		});

		return found;
	}

	return true;
}

