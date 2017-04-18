#include "visible_entities.h"

#include "game/transcendental/cosmos.h"
#include "game/components/render_component.h"

#include "game/systems_inferred/tree_of_npo_system.h"
#include "game/systems_inferred/physics_system.h"
#include "game/systems_stateless/render_system.h"

#include "game/enums/filters.h"

visible_entities::visible_entities(const camera_cone camera, const cosmos& cosmos) {
	from_camera(camera, cosmos);
}

void visible_entities::from_camera(
	const camera_cone camera,
	const cosmos& cosmos
) {
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

	render_system().get_visible_per_layer(
		cosmos,
		all,
		per_layer
	);
}