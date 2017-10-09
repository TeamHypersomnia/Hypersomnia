#include "workspace.h"

#include "game/organization/all_messages_includes.h"
#include "game/organization/all_component_includes.h"

#include "test_scenes/test_scenes_content.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"

#if BUILD_TEST_SCENES
void workspace::make_test_scene(sol::state& lua, const bool minimal) {
	world.clear();
	logicals = {};
	viewables = {};
	world.reserve_storage_for_entities(3000u);

	populate_test_scene_assets(lua, logicals, viewables);

	if (minimal) {
		test_scenes::minimal_scene().populate_world_with_entities(world, logicals);
	}
	else {
		test_scenes::testbed().populate_world_with_entities(world, logicals);
	}

	locally_viewed = world.get_entity_by_name(L"player0");
}
#endif

void workspace::make_blank() {
	world.clear();
	world.reserve_storage_for_entities(100);

	auto origin = world.create_entity("origin_entity");
	origin += components::transform();

	locally_viewed = origin;
}