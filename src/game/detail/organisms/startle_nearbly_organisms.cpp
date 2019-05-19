#include "augs/math/camera_cone.h"
#include "game/detail/visible_entities.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/components/movement_path_component.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

void startle_nearby_organisms(
	cosmos& cosm,
	const vec2 startle_origin,
	const real32 startle_radius,
	const real32 startle_force,
	const startle_type type,
	const render_layer_filter organism_types
) {
	auto& neighbors = thread_local_visible_entities();

	neighbors.clear();
	neighbors.acquire_non_physical({
		cosm,
		{ camera_eye(startle_origin), vec2::square(startle_radius * 2) },
		visible_entities_query::accuracy_type::PROXIMATE,
		organism_types,
		{ tree_of_npo_type::ORGANISMS }
	});

	neighbors.for_all(cosm, [&](const auto handle) {
		handle.template dispatch_on_having_all<components::movement_path>([&](const auto typed_neighbor) {
			const auto neighbor_tip = *typed_neighbor.find_logical_tip();
			const auto target_offset = neighbor_tip - startle_origin;
			const auto target_dist = target_offset.length();

			const auto startle_amount = startle_radius - target_dist;

			if (startle_amount > 0.f) {
				const auto target_dir = target_offset / target_dist;
				const auto startle_impulse = target_dir * startle_amount * startle_force;

				typed_neighbor.template get<components::movement_path>().add_startle(type, startle_impulse);
			}
		});
	});
}
