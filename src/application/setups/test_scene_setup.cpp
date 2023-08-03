#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/test_scene_setup.h"

#include "application/arena/choose_arena.h"
#include "application/setups/editor/packaged_official_content.h"

test_scene_setup::test_scene_setup(
	sol::state& lua,
	std::string nickname,
	const packaged_official_content& official,
	const test_scene_settings settings,
	const input_recording_type recording_type,
	const test_scene_type type
) : type(type) {
#if BUILD_TEST_SCENES
	scene.make_test_scene(lua, settings);
	auto& cosm = scene.world;

	editor_project project;

	//current_arena_folder = "user/projects/shooting_range";
	if (type == test_scene_type::TUTORIAL) {
		current_arena_folder = "content/menu/tutorial";
	}
	else {
		current_arena_folder = "content/menu/shooting_range";
	}

	auto paths = editor_project_paths(current_arena_folder);

	::load_arena_from_path(
		{
			lua,
			get_arena_handle(),
			official,
			"",
			"",
			clean_round_state,
			std::nullopt,
			&project
		},

		paths.project_json,
		nullptr
	);

	get_arena_handle().on_mode_with_input(
		[&]<typename M>(M& mode, const auto& input) {
			if constexpr(std::is_same_v<test_mode, M>) {
				for (auto& p : project.nodes.template get_pool_for<editor_point_marker_node>()) {
					if (p.editable.faction == faction_type::RESISTANCE) {
						auto new_id = mode.add_player(input, nickname, faction_type::RESISTANCE);
						mode.find(new_id)->dedicated_spawn = p.scene_entity_id;
						mode.find(new_id)->hide_in_scoreboard = true;
						mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
					}
				}

				local_player_id = mode.add_player(input, nickname, faction_type::METROPOLIS);
				viewed_character_id = cosm[mode.lookup(local_player_id)].get_id();

				mode.infinite_ammo_for = viewed_character_id;
			}
			else {
				ensure("Unsupported mode." && false);
			}
		}
	);
#else
	(void)lua;
	(void)settings;
#endif

	if (recording_type != input_recording_type::DISABLED) {
		//if (player.try_to_load_or_save_new_session(USER_FILES_DIR "/sessions/", "recorded.inputs")) {
		//
		//}
	}

	/* Close any window that might be open by default */
	escape();
}

void test_scene_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Hypersomnia test scene";

	if (speed < 1.0f) {
		//config.interpolation.enabled = false;
	}

	if (is_tutorial()) {
		auto& mult = config.session.camera_query_aabb_mult;

		if (std::clamp(mult, 1.0f, 1.7f) == mult) {
			/* We want the first impression to be good so increase it in case there are any glitches. */
			mult = 1.5f;
		}

		if (tutorial.level == 0) {
			config.drawing.draw_hotbar = false;
		}
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

void test_scene_setup::draw_custom_gui(const draw_setup_gui_input& in) { 
	arena_gui_base::draw_custom_gui(in);
}

setup_escape_result test_scene_setup::escape() {
	return arena_gui_base::escape();
}

bool test_scene_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (arena_gui_base::handle_input_before_game(in)) {
		return true;
	}

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

test_arena_handle<false> test_scene_setup::get_arena_handle() {
	return get_arena_handle_impl<test_arena_handle<false>>(*this);
}

test_arena_handle<true> test_scene_setup::get_arena_handle() const {
	return get_arena_handle_impl<test_arena_handle<true>>(*this);
}