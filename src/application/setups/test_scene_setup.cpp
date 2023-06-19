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
	scene.make_test_scene(lua, settings);
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

	if (speed < 1.0f) {
		config.interpolation.enabled = false;
	}
}

void test_scene_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

bool test_scene_setup::handle_input_before_imgui(
	handle_input_before_imgui_input
) {
	using namespace augs::event;

	return false;
}

bool test_scene_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	auto set_speed = [&](const auto s) {
		speed = s;
	};

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (in.e.was_any_key_pressed()) {
			switch (key) {
				case key::NUMPAD0: set_speed(1.0); return true;
				case key::NUMPAD1: set_speed(0.01); return true;
				case key::NUMPAD2: set_speed(0.05); return true;
				case key::NUMPAD3: set_speed(0.1); return true;
				case key::NUMPAD4: set_speed(0.5); return true;
				case key::NUMPAD5: set_speed(2.0); return true;
				case key::NUMPAD6: set_speed(4.0); return true;
				case key::NUMPAD7: set_speed(10.0); return true;
				default: break;
			}
		}
	}

	return false;
}