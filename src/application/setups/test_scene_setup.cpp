#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/test_scene_setup.h"

test_scene_setup::test_scene_setup(
	sol::state& lua,
	const test_scene_settings settings,
	const input_recording_type recording_type
) {
#if BUILD_TEST_SCENES
	scene.make_test_scene(lua, settings, ruleset);
	auto& cosm = scene.world;

	local_player_id = mode.add_player({ ruleset, cosm }, "Player", faction_type::METROPOLIS);
	viewed_character_id = cosm[mode.lookup(local_player_id)].get_id();
#else
	(void)lua;
	(void)settings;
#endif

	if (recording_type != input_recording_type::DISABLED) {
		//if (player.try_to_load_or_save_new_session(USER_FILES_DIR "/sessions/", "recorded.inputs")) {
		//
		//}
	}
}

void test_scene_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Hypersomnia test scene";
}

void test_scene_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}