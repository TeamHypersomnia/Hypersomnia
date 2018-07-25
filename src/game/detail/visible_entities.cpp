#include "augs/math/math.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/templates/container_templates.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/visible_entities.h"

#include "game/cosmos/cosmos.h"
#include "game/components/render_component.h"

#include "game/enums/filters.h"
#include "game/detail/physics/physics_scripts.h"

#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/inferred_caches/physics_world_cache.h"

#include "game/detail/passes_filter.h"
#include "game/detail/calc_render_layer.h"
#include "augs/templates/enum_introspect.h"

static constexpr auto EXACT = visible_entities_query::accuracy_type::EXACT;

visible_entities::visible_entities(const visible_entities_query input) {
	reacquire_all_and_sort(input);
}

void visible_entities::clear() {
	for (auto& layer : per_layer) {
		layer.clear();
	}
}

visible_entities& visible_entities::reacquire_all_and_sort(const visible_entities_query input) {
	clear();
	acquire_non_physical(input);
	acquire_physical(input);
	sort_car_interiors(input.cosm);

	return *this;
}

void visible_entities::acquire_physical(const visible_entities_query input) {
	const auto& cosmos = input.cosm;
	const auto camera = input.cone;

	const auto& physics = cosmos.get_solvable_inferred().physics;

	thread_local std::unordered_set<entity_id> unique_from_physics;
	unique_from_physics.clear();

	if (input.accuracy == EXACT) {
		const auto camera_aabb = camera.get_visible_world_rect_aabb();
		
		physics.for_each_intersection_with_polygon(
			cosmos.get_si(),
			camera_aabb.get_vertices<real32>(),
			filters::renderable_query(),
			[&](const b2Fixture* const fix, auto, auto) {
				const auto owning_entity_id = cosmos.to_versioned(get_entity_that_owns(fix));

				if (::passes_filter(input.filter, cosmos, owning_entity_id)) {
					unique_from_physics.insert(owning_entity_id);
				}

				return callback_result::CONTINUE;
			}
		);
	}
	else {
		physics.for_each_in_camera(
			cosmos.get_si(),
			camera,
			[&](const b2Fixture* const fix) {
				const auto owning_entity_id = cosmos.to_versioned(get_entity_that_owns(fix));

				if (::passes_filter(input.filter, cosmos, owning_entity_id)) {
					unique_from_physics.insert(owning_entity_id);
				}

				return callback_result::CONTINUE;
			}
		);
	}

	for (const auto& a : unique_from_physics) {
		register_visible(cosmos, a);
	}
}

void visible_entities::acquire_non_physical(const visible_entities_query input) {
	const auto& cosmos = input.cosm;
	const auto camera = input.cone;
	const auto camera_aabb = camera.get_visible_world_rect_aabb();

	const auto& tree_of_npo = cosmos.get_solvable_inferred().tree_of_npo;
	
	auto acquire_from = [&](const tree_of_npo_type type) {
		tree_of_npo.for_each_in_camera(
			[&](const unversioned_entity_id unversioned_id) {
				const auto id = cosmos.to_versioned(unversioned_id);
				
				if (!::passes_filter(input.filter, cosmos, id)) {
					return;
				}

				if (input.accuracy == EXACT) {
					const bool visible = cosmos[id].dispatch([&](const auto typed_handle) {
						const auto aabb = typed_handle.find_aabb();

						if (aabb == std::nullopt) {
							return false;
						}

						if (!camera_aabb.hover(*aabb)) {
							return false;
						}

						if (camera.screen_size == vec2i::square(1)) {
							/* This is an infinitely small point. */
							if (const auto transform = typed_handle.find_logic_transform()) {
								const auto size = typed_handle.get_logical_size();

								if (!point_in_rect(
									transform->pos,
									transform->rotation,
									size,
									camera.eye.transform.pos
								)) {
									return false;
								}
							}
							else {
								return false;
							}
						}

						return true;
					});

					if (visible) {
						register_visible(cosmos, id);
					}
				}
				else {
					register_visible(cosmos, id);
				}
			},
			camera,
			type
		);
	};

	augs::for_each_enum_except_bounds(
		[&](const tree_of_npo_type type){ 
			if (input.types.types[type]) {
				acquire_from(type);
			}
		}
	);
}

void visible_entities::clear_dead_entities(const cosmos& cosm) {
	auto dead_deleter = [&cosm](const entity_id e) {
		return cosm[e].dead();
	};

	for (auto& layer : per_layer) {
		erase_if(layer, dead_deleter);
	}
}

void visible_entities::register_visible(const cosmos& cosm, const entity_id id) {
	per_layer[::calc_render_layer(cosm[id])].push_back(id);
}

void visible_entities::sort_car_interiors(const cosmos& cosm) {
	auto& car_interior_layer = per_layer[render_layer::CAR_INTERIOR];

	if (car_interior_layer.size() > 1) {
		sort_range(
			car_interior_layer, 
			[&cosm](const auto b, const auto a) {
				return are_connected_by_friction(cosm[a], cosm[b]);
			}
		);
	}
}