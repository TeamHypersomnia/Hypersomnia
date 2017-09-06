#include "game/detail/visible_entities.h"

#include "game/transcendental/cosmos.h"
#include "game/components/render_component.h"

#include "game/enums/filters.h"
#include "game/detail/physics/physics_scripts.h"

#include "game/systems_inferred/tree_of_npo_system.h"
#include "game/systems_inferred/physics_system.h"

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

	const auto& tree_of_npo = cosmos.systems_inferred.get<tree_of_npo_system>();
	const auto& physics = cosmos.systems_inferred.get<physics_system>();

	all.clear();

	for (auto& layer : per_layer) {
		layer.clear();
	}

	tree_of_npo.determine_visible_entities_from_camera(
		all, 
		camera
	);

	thread_local std::unordered_set<unversioned_entity_id> unique_from_physics;
	unique_from_physics.clear();

	physics.for_each_in_camera(
		cosmos.get_si(),
		camera,
		[&](const auto fix) {
			unique_from_physics.insert(get_id_of_entity_of_fixture(fix));
			return callback_result::CONTINUE;
		}
	);

	concatenate(all, unique_from_physics);
	get_visible_per_layer(cosmos, all, per_layer);
}