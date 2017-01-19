#include "visible_entities.h"

#include "game/transcendental/cosmos.h"

#include "game/systems_temporary/dynamic_tree_system.h"
#include "game/systems_temporary/physics_system.h"
#include "game/systems_stateless/render_system.h"

visible_entities::visible_entities(const camera_cone camera, const cosmos& cosmos) {
	const auto& dynamic_tree = cosmos.systems_temporary.get<dynamic_tree_system>();
	const auto& physics = cosmos.systems_temporary.get<physics_system>();

	auto all_visible = dynamic_tree.determine_visible_entities_from_camera(camera);
	const auto visible_from_physics = physics.query_camera(camera).entities;

	all_visible.insert(all_visible.end(), visible_from_physics.begin(), visible_from_physics.end());

	all = cosmos[all_visible];
	per_layer = render_system().get_visible_per_layer(all);
}