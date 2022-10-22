#include "augs/math/math.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/templates/container_templates.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/visible_entities.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/render_component.h"

#include "game/enums/filters.h"
#include "game/detail/physics/physics_scripts.h"

#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/inferred_caches/organism_cache_query.hpp"
#include "game/inferred_caches/organism_cache.hpp"

#include "game/detail/passes_filter.h"
#include "game/detail/calc_render_layer.h"
#include "game/detail/calc_sorting_order.h"
#include "augs/templates/enum_introspect.h"

static bool is_icon_type(const tree_of_npo_type type) {
	switch (type) {
		case tree_of_npo_type::RENDERABLES:
		case tree_of_npo_type::ORGANISMS:
			return false;
		default:
			return true;
	}
}

std::size_t visible_entities::count_all() const {
	return ::accumulate_sizes(per_layer);
}

void visible_entities::layer_register::clear() {
	with_orders.clear();
}

void visible_entities::layer_register::sort() {
	sort_range(with_orders);
}

void visible_entities::clear() {
	for (auto& layer : per_layer) {
		layer.clear();
	}

	for (auto& f : per_function) {
		f.clear();
	}
}

visible_entities& visible_entities::reacquire_all(const visible_entities_query input) {
	clear();
	acquire_non_physical(input);
	acquire_physical(input);

	return *this;
}

void visible_entities::sort(const cosmos& cosm) {
	sort_car_interiors(cosm);

	for (auto& layer : per_layer) {
		layer.sort();
	}
}

/* We're using our own flags instead of unordered_set implementation for it to be deterministic */

template <class T>
using make_flags = std::array<bool, T::statically_allocated_entities>;

using all_flags = per_entity_type_container<make_flags>;

void visible_entities::acquire_physical(const visible_entities_query input) {
	const auto& cosm = input.cosm;
	const auto camera = input.cone;

	const auto& physics = cosm.get_solvable_inferred().physics;

	thread_local auto unique_flags = all_flags();
	thread_local auto unique_from_physics = std::vector<entity_id>();
	unique_from_physics.clear();

	auto get_flag_for = [&](const entity_id& e) -> bool& {
		return unique_flags.visit(e.type_id, [&](auto& typed_flags) -> bool& {
			return typed_flags[e.raw.indirection_index];
		});
	};

	auto unset_all_on_exit = augs::scope_guard(
		[&]() {
			for (const auto& a : unique_from_physics) {
				register_visible(cosm, a);
				get_flag_for(a) = false;
			}
		}
	);

	auto register_unique = [&](const entity_id& e) {
		auto& f = get_flag_for(e);

		if (!f) {
			unique_from_physics.push_back(e);
			f = true;
		}
	};

	if (input.accuracy == accuracy_type::EXACT) {
		const auto camera_aabb = camera.get_visible_world_rect_aabb();
		
		physics.for_each_intersection_with_polygon(
			cosm.get_si(),
			camera_aabb.get_vertices(),
			predefined_queries::renderable(),
			[&](const b2Fixture& fix, auto, auto) {
				const auto owning_entity_id = cosm.get_versioned(get_entity_that_owns(fix));

				if (::passes_filter(input.filter, cosm, owning_entity_id)) {
					register_unique(owning_entity_id);
				}

				return callback_result::CONTINUE;
			}
		);
	}
	else {
		physics.for_each_in_camera(
			cosm.get_si(),
			camera,
			[&](const b2Fixture& fix) {
				const auto owning_entity_id = cosm.get_versioned(get_entity_that_owns(fix));
				const auto handle = cosm[owning_entity_id];

				handle.dispatch(
					[&](const auto& typed_handle) {
						using T = remove_cref<decltype(typed_handle)>;

						auto add_if_passes = [&](const auto& what) {
							if (::passes_filter(input.filter, what)) {
								register_unique(what.get_id());
							}
						};

						if constexpr(T::template has<components::item>()) {
							if (const auto owning_capability = typed_handle.get_owning_transfer_capability()) {
								add_if_passes(owning_capability);
							}
						}

						add_if_passes(typed_handle);
					}
				);

				return callback_result::CONTINUE;
			}
		);
	}
}

void visible_entities::acquire_non_physical(const visible_entities_query input) {
	const auto& cosm = input.cosm;
	const auto camera = input.cone;
	const auto camera_aabb = camera.get_visible_world_rect_aabb();

	const auto& tree_of_npo = cosm.get_solvable_inferred().tree_of_npo;
	const auto& organisms = cosm.get_solvable_inferred().organisms;
	
	auto acquire_from = [&](const tree_of_npo_type type) {
		auto add_visible = [&](const auto& id) {
			if (!::passes_filter(input.filter, cosm, id)) {
				return;
			}

			if (input.accuracy == accuracy_type::PROXIMATE) {
				register_visible(cosm, id);
				return;
			}

			ensure(input.accuracy == accuracy_type::EXACT)

			const bool visible = cosm[id].dispatch([&](const auto typed_handle) {
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
				register_visible(cosm, id);
			}
		};

		const bool add_all_iconed = 
			is_icon_type(type) && input.types.force_add_all_icons
		;

		if (add_all_iconed) {
			tree_of_npo.for_all_of_type(
				[&](const auto& unversioned_id) {
					const auto id = cosm.get_versioned(unversioned_id);
					//if (::passes_filter(input.filter, cosm, id)) {
						register_visible(cosm, id);
						//}
				},
				type
			);

			return;
		}

		if (type == tree_of_npo_type::ORGANISMS) {
			organisms.for_each_cell_of_all_grids(
				camera.get_visible_world_rect_aabb(),
				[&](const auto& cell) {
					for (const auto& o : cell.organisms) {
						add_visible(o);
					}
				}
			);
		}
		else {
			tree_of_npo.for_each_in_camera(
				[&](const auto& unversioned_id) {
					const auto id = cosm.get_versioned(unversioned_id);
					add_visible(id);
				},
				camera,
				type
			);
		}
	};

	augs::for_each_enum_except_bounds(
		[&](const tree_of_npo_type type){ 
			if (input.types.types[type]) {
				acquire_from(type);
			}
		}
	);
}

void visible_entities::layer_register::register_visible(const entity_id id, const sorting_order_type order) {
	with_orders.emplace_back(order, id);
}

void visible_entities::register_visible(const cosmos& cosm, const entity_id id) {
	cosm[id].template conditional_dispatch_ret<entities_with_render_layer>(
		[&](const auto& typed_handle) {
			if constexpr(!is_nullopt_v<decltype(typed_handle)>) {
				const auto layer = ::calc_render_layer(typed_handle);
				const auto order = ::calc_sorting_order(typed_handle);

				per_layer[layer].register_visible(id, order);

				using E = remove_cref<decltype(typed_handle)>;

				if constexpr(E::template has<invariants::render>()) {
					const auto& functions = typed_handle.template get<invariants::render>().special_functions;

					for (int i = 0; i < static_cast<int>(functions.size()); ++i) {
						if (functions[i]) {
							per_function[i].emplace_back(id);
						}
					}
				}
			}
		}
	);
}

std::size_t visible_entities::layer_register::size() const {
	return with_orders.size();
}

void visible_entities::sort_car_interiors(const cosmos& cosm) {
#if TODO_CARS
	auto& car_interior_layer = per_layer[render_layer::CAR_INTERIOR];

	if (car_interior_layer.size() > 1) {
		sort_range(
			car_interior_layer, 
			[&cosm](const auto b, const auto a) {
				return are_connected_by_friction(cosm[a], cosm[b]);
			}
		);
	}
#else
	(void)cosm;
#endif
}