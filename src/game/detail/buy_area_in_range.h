#pragma once
#include "game/detail/visible_entities.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/physics/shape_overlapping.hpp"
#include "game/detail/entity_handle_mixins/get_owning_transfer_capability.hpp"

template <class E>
bool buy_area_in_range(const E& subject) {
	const auto& cosm = subject.get_cosmos();

	if (!::sentient_and_conscious(subject)) {
		return false;
	}

	const auto capability = subject.get_owning_transfer_capability();
	const auto matched_faction = capability.alive() ? capability.get_official_faction() : faction_type::SPECTATOR;

	auto& entities = thread_local_visible_entities();

	entities.reacquire_all({
		cosm,
		camera_cone::from_aabb(subject),
		accuracy_type::PROXIMATE,
		render_layer_filter::whitelist(render_layer::AREA_MARKERS),
		{ { tree_of_npo_type::RENDERABLES } }
	});

	entities.sort(cosm);

	bool found = false;

	entities.for_each<render_layer::AREA_MARKERS>(cosm, [&](const auto& handle) {
		return handle.template dispatch_on_having_all_ret<invariants::area_marker>([&](const auto& typed_handle) {
			if constexpr(is_nullopt_v<decltype(typed_handle)>) {
				return callback_result::CONTINUE;
			}
			else {
				const auto& marker = typed_handle.template get<invariants::area_marker>();

				if (area_marker_type::BUY_ZONE == marker.type) {
					if (matched_faction == typed_handle.get_official_faction()) {
						if (entity_overlaps_entity(typed_handle, subject)) {
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

