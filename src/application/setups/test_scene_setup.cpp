#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/test_scene_setup.h"

#include "application/arena/choose_arena.h"
#include "application/setups/editor/packaged_official_content.h"

#include "game/messages/game_notification.h"
#include "application/setups/editor/project/editor_project.hpp"
#include "augs/string/typesafe_sscanf.h"

using portal_id = editor_typed_node_id<editor_area_marker_node>;

auto make_portal_id_type(const editor_node_id id) {
	if (id.type_id.is<editor_area_marker_node>()) {
		return portal_id::from_generic(id);
	}

	return portal_id();
}

test_scene_setup::test_scene_setup(
	sol::state& lua,
	std::string nickname,
	const packaged_official_content& official,
	const test_scene_settings settings,
	const input_recording_type recording_type,
	const test_scene_type type
) : nickname(nickname), type(type) {
	scene.make_test_scene(lua, settings);

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
		nullptr,
		std::addressof(entity_to_node)
	);

	clean_mode_state = current_mode_state;
	name_to_node = project.make_name_to_node_map();

	restart_mode();

	if (recording_type != input_recording_type::DISABLED) {
		//if (player.try_to_load_or_save_new_session(USER_FILES_DIR "/sessions/", "recorded.inputs")) {
		//
		//}
	}

	/* Close any window that might be open by default */
	escape();
}

void test_scene_setup::restart_arena() {
	LOG("Setting up tutorial level: %x.", tutorial.level);

	auto& cosm = scene.world;

	auto character = [&]() {
		return cosm[get_controlled_character_id()];
	};

	auto pre_crosshair = character().get<components::crosshair>();
	auto pre_movement_flags = character().get<components::movement>().flags;

	cosm.set(clean_round_state);
	current_mode_state = clean_mode_state;

	restart_mode();

	character().get<components::crosshair>() = pre_crosshair;
	character().get<components::movement>().flags = pre_movement_flags;
}

void test_scene_setup::restart_mode() {
	auto& cosm = scene.world;

	const auto current_teleport = typesafe_sprintf("entry%x", tutorial.level);

	get_arena_handle().on_mode_with_input(
		[&]<typename M>(M& mode, const auto& input) {
			if constexpr(std::is_same_v<test_mode, M>) {
				for (auto& p : project.nodes.template get_pool_for<editor_point_marker_node>()) {
					if (p.editable.faction == faction_type::RESISTANCE) {
						const auto new_id = mode.add_player(input, nickname, faction_type::RESISTANCE);
						mode.find(new_id)->dedicated_spawn = p.scene_entity_id;
						mode.find(new_id)->hide_in_scoreboard = true;
						mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
					}
				}

				local_player_id = mode.add_player(input, nickname, faction_type::METROPOLIS);
				viewed_character_id = cosm[mode.lookup(local_player_id)].get_id();

				const auto new_id = local_player_id;
				auto player = mode.find(new_id);

				ensure(player != nullptr)

				if (const auto tp = project.find_node(make_portal_id_type(name_to_node[current_teleport]))) {
					player->dedicated_spawn = tp->scene_entity_id;
					mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
				}

				mode.infinite_ammo_for = viewed_character_id;
			}
			else {
				ensure("Unsupported mode." && false);
			}
		}
	);
}


bool test_scene_setup::post_solve(const const_logic_step step) {
	const auto& notifications = step.get_queue<messages::game_notification>();

	auto& cosm = scene.world;

	if (cosm.get_total_steps_passed() <= clean_round_state.clk.now.step + 1) {
		/* Otherwise we'd have an infinite loop. */
		return false;
	}

	for (const auto& n : notifications) {
		if (const auto tp = std::get_if<messages::teleportation>(std::addressof(n.payload))) {
			if (tp->teleported == viewed_character_id) {
				const auto node_id = ::entity_to_node_id(entity_to_node, tp->to_portal);

				if (const auto portal = project.find_node(make_portal_id_type(node_id))) {
					const auto& name = portal->unique_name;

					if (begins_with(name, "entry")) {
						uint32_t new_level = 0;

						if (typesafe_sscanf(name, "entry%x", new_level)) {
							tutorial.level = new_level;
							restart_arena();
							return true;
						}
					}
				}
			}
		}
	}

	return false;
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

		if (tutorial.level < 10) {
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