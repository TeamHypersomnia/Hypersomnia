#include "game/detail/visible_entities.h"

#include "game/transcendental/cosmos.h"
#include "game/components/render_component.h"

#include "game/enums/filters.h"
#include "game/detail/physics/physics_scripts.h"

#include "game/inferential_systems/tree_of_npo_system.h"
#include "game/inferential_systems/physics_system.h"

static void get_visible_per_layer(
	const cosmos& cosmos,
	const visible_entities::all_type& entities,
	visible_entities::per_layer_type& output_layers
) {
	if (entities.empty()) {
		return;
	}

	for (const auto it_id : entities) {
		const auto it = cosmos[it_id];
		const auto layer = it.get<components::render>().layer;
		ensure(layer < static_cast<render_layer>(output_layers.size()));
		output_layers[layer].push_back(it);
	}

	auto& car_interior_layer = output_layers[render_layer::CAR_INTERIOR];

	if (car_interior_layer.size() > 1) {
		sort_container(
			car_interior_layer, 
			[&cosmos](const auto b, const auto a) {
				return are_connected_by_friction(cosmos[a], cosmos[b]);
			}
		);
	}
}

visible_entities::visible_entities(const visible_entities_query input) {
	reacquire(input);
}

void visible_entities::reacquire(const visible_entities_query input) {
	const auto& cosmos = input.cosm;
	const auto camera = input.cone;

	const auto& tree_of_npo = cosmos.inferential.get<tree_of_npo_system>();
	const auto& physics = cosmos.inferential.get<physics_system>();

	all.clear();

	for (auto& layer : per_layer) {
		layer.clear();
	}

	tree_of_npo.for_each_visible_in_camera(
		[this, &cosmos](const unversioned_entity_id id) {
			all.push_back(cosmos.make_versioned(id));
		},
		camera,
		tree_of_npo_type::RENDERABLES
	);

	thread_local std::unordered_set<entity_id> unique_from_physics;
	unique_from_physics.clear();

	physics.for_each_in_camera(
		cosmos.get_si(),
		camera,
		[&](const b2Fixture* const fix) {
			unique_from_physics.insert(cosmos.make_versioned(get_entity_that_owns(fix)));
			return callback_result::CONTINUE;
		}
	);

	concatenate(all, unique_from_physics);
	get_visible_per_layer(cosmos, all, per_layer);
}

void visible_entities::clear_dead(const cosmos& cosm) {
	auto dead_deleter = [&cosm](const entity_id e) {
		return cosm[e].dead();
	};

	erase_if(all, dead_deleter);

	for (auto& layer : per_layer) {
		erase_if(layer, dead_deleter);
	}
}