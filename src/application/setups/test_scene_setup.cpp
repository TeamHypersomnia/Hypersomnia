#include "game/transcendental/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "test_scenes/test_scenes_content.h"
#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"

#include "application/setups/test_scene_setup.h"

#include "generated/introspectors.h"

using namespace augs::event::keys;

test_scene_setup::test_scene_setup(
	sol::state& lua,
	const bool make_minimal_test_scene,
	const input_recording_type recording_type
) {
	hypersomnia.set_steps_per_second(60);
#if BUILD_TEST_SCENES
	hypersomnia.reserve_storage_for_entities(3000u);

	populate_test_scene_assets(lua, logical_assets, viewable_defs);

	if (make_minimal_test_scene) {
		test_scenes::minimal_scene().populate_world_with_entities(
			hypersomnia,
			logical_assets
		);
	}
	else {
		test_scenes::testbed().populate_world_with_entities(
			hypersomnia,
			logical_assets
		);
	}

	characters.acquire_available_characters(hypersomnia);
#endif

	if (recording_type != input_recording_type::DISABLED) {
		if (player.try_to_load_or_save_new_session("generated/sessions/", "recorded.inputs")) {

		}
	}
}

void test_scene_setup::control(const cosmic_entropy& events) {
	total_collected_entropy += events;
}

void test_scene_setup::accept_game_gui_events(const cosmic_entropy& events) {
	total_collected_entropy += events;
}