#include "visible_entities.h"

#include "game/transcendental/cosmos.h"

#include "game/systems_temporary/dynamic_tree_system.h"
#include "game/systems_temporary/physics_system.h"
#include "game/systems_stateless/render_system.h"

#include "game/enums/filters.h"

visible_entities::visible_entities(const camera_cone camera, const cosmos& cosmos) {
	from_camera(camera, cosmos);
}

void visible_entities::from_camera(
	const camera_cone camera,
	const cosmos& cosmos
) {
	const auto& dynamic_tree = cosmos.systems_temporary.get<dynamic_tree_system>();
	const auto& physics = cosmos.systems_temporary.get<physics_system>();

	all.clear();

	for (auto& layer : per_layer) {
		layer.clear();
	}

	dynamic_tree.determine_visible_entities_from_camera(
		all, 
		camera
	);

	static thread_local std::unordered_set<unversioned_entity_id> unique_from_physics;
	unique_from_physics.clear();

	physics.for_each_in_camera(
		camera,
		[&](const auto fix) {
			unique_from_physics.insert(get_id_of_entity_of_fixture(fix));
			return query_callback_result::CONTINUE;
		}
	);

	concatenate(all, unique_from_physics);

	render_system().get_visible_per_layer(
		cosmos,
		all,
		per_layer
	);
}