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
#include "augs/gui/text/printer.h"

#include "game/modes/detail/delete_with_held_items.hpp"

using portal_marker = editor_area_marker_node;

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

	auto json_read_settings = editor_project_readwrite::reading_settings();
	json_read_settings.read_inactive_nodes = false;

	::load_arena_from_path(
		{
			json_read_settings,
			lua,
			get_arena_handle(),
			official,
			"",
			"",
			clean_round_state,
			std::nullopt,
			&project,
			std::addressof(entity_to_node)
		},

		paths.project_json,
		nullptr
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

template <class T>
const T* test_scene_setup::find(const entity_id& id) const {
	return find<T>(::entity_to_node_id(entity_to_node, id));
}

template <class T>
const T* test_scene_setup::find(const editor_node_id& generic_id) const {
	return project.find_node<T>(generic_id);
}

template <class T>
const T* test_scene_setup::find(const std::string& name) const {
	if (const auto generic_id = mapped_or_nullptr(name_to_node, name)) {
		return find<T>(*generic_id);
	}

	return nullptr;
}

entity_handle test_scene_setup::to_handle(const std::string& name) {
	return scene.world[to_entity(name)];
}

const_entity_handle test_scene_setup::to_handle(const std::string& name) const{
	return scene.world[to_entity(name)];
}

entity_id test_scene_setup::to_entity(const std::string& name) const {
	if (const auto opp = mapped_or_nullptr(opponents, name)) {
		return *opp;
	}

	if (const auto generic_id = mapped_or_nullptr(name_to_node, name)) {
		entity_id id;

		project.on_node(
			*generic_id,
			[&](const auto& typed_node, const auto&) {
				id = typed_node.scene_entity_id;
			}
		);

		return id;
	}

	return {};
}

bool test_scene_setup::is_killed(const std::string& name) const {
	if (auto h = to_handle(name)) {
		if (auto sent = h.template find<components::sentience>()) {
			return !sent->is_conscious();
		}
	}

	return false;
}

void test_scene_setup::remove(logic_step step, const std::string& name) {
	if (auto h = to_handle(name)) {
		if (h.template has<components::sentience>()) {
			::delete_with_held_items_except({}, step, h);
		}
		else {
			step.queue_deletion_of(h, "test_scene_setup::remove");
		}
	}
}

void test_scene_setup::restart_mode() {
	auto& cosm = scene.world;

	const auto current_teleport = typesafe_sprintf("entry%x", tutorial.level);

	const bool is_akimbo_level = tutorial.level == 5;
	const bool is_duals_level = tutorial.level == 6;
	const bool is_ricochets_level = tutorial.level == 8;
	const bool is_try_throwing_reloading_level = tutorial.level == 10 || tutorial.level == 13;

	get_arena_handle().on_mode_with_input(
		[&]<typename M>(M& mode, const auto& input) {
			if constexpr(std::is_same_v<test_mode, M>) {
				for (auto& p : project.nodes.template get_pool_for<editor_point_marker_node>()) {
					if (p.editable.faction == faction_type::RESISTANCE) {
						const auto new_id = mode.add_player(input, nickname, faction_type::RESISTANCE);
						mode.find(new_id)->dedicated_spawn = p.scene_entity_id;
						mode.find(new_id)->hide_in_scoreboard = true;
						const auto opponent_id = mode.find(new_id)->controlled_character_id;
						mode.teleport_to_next_spawn(input, new_id, opponent_id);

						if (is_tutorial()) {
							mode.find(new_id)->allow_respawn = is_akimbo_level;
						}

						opponents[p.unique_name] = opponent_id;
					}
				}

				local_player_id = mode.add_player(input, nickname, faction_type::METROPOLIS);
				viewed_character_id = cosm[mode.lookup(local_player_id)].get_id();

				const auto new_id = local_player_id;
				auto player = mode.find(new_id);

				ensure(player != nullptr)

				if (const auto tp = find<portal_marker>(current_teleport)) {
					player->dedicated_spawn = tp->scene_entity_id;
					mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
				}

				if (!is_tutorial() || is_akimbo_level || is_duals_level || is_ricochets_level || is_try_throwing_reloading_level) {
					mode.infinite_ammo_for = viewed_character_id;
				}
			}
			else {
				ensure("Unsupported mode." && false);
			}
		}
	);
}

void test_scene_setup::do_tutorial_logic(const logic_step step) {
	(void)step;

	auto get_opp = [&](int i, int j) {
		if (j == 0) {
			return std::string("kill") + std::to_string(i);
		}

		return typesafe_sprintf("kill%x (%x)", i, j);
	};

	auto exists = [&](int i, int j) {
		return to_entity(get_opp(i, j)) != entity_id();
	};

	auto killed = [&](int i, int j) {
		return is_killed(get_opp(i, j));
	};

	auto obs = [&](int i, int j = 0) {
		if (j == 0) {
			return std::string("obs") + std::to_string(i);
		}

		return typesafe_sprintf("obs%x (%x)", i, j);
	};

	{
		int j = 0;

		const auto only_headshot_enemies_index = 13;

		while (exists(only_headshot_enemies_index, j)) {
			if (auto handle = to_handle(get_opp(only_headshot_enemies_index, j))) {
				if (auto sent = handle.template find<components::sentience>()) {
					auto& hp = sent->get<health_meter_instance>();

					if (hp.value > 0) {
						hp.value = hp.maximum;

						auto& mp = sent->get<personal_electricity_meter_instance>();
						mp.value = 10;
					}
				}
			}

			++j;
		}

	}

	for (int i = 1;; ++i) {
		if (!exists(i, 0)) {
			break;
		}

		bool all_killed = true;

		int j = 0;

		while (exists(i, j)) {
			if (!killed(i, j)) {
				if (const auto disable_armor = i == 14 || i == 16) {
					if (auto handle = to_handle(get_opp(i, j))) {
						if (auto armor = handle[slot_function::TORSO_ARMOR].get_item_if_any()) {
							step.queue_deletion_of(armor, "disable armor");
						}
					}
				}

				all_killed = false;
			}

			++j;
		}

		if (all_killed) {
			remove(step, obs(i));

			const bool is_akimbo_level = tutorial.level == 5;
			//const bool is_double_throw_stage = i == 10;

			const bool have_to_kill_simultaneously = is_akimbo_level;

			if (have_to_kill_simultaneously) {
				for (int jj = 0; jj < j; ++jj) {
					remove(step, get_opp(i, jj));
				}
			}

			int k = 0;

			while (to_entity(obs(i, k)).is_set()) {
				remove(step, obs(i, k));
				++k;
			}
		}
	}
}

void test_scene_setup::pre_solve(const logic_step step) {
	do_tutorial_logic(step);
}

bool test_scene_setup::post_solve(const const_logic_step step) {
	const auto& notifications = step.get_queue<messages::game_notification>();

	if (restart_requested) {
		restart_requested = false;

		restart_arena();
		return true;
	}

	auto& cosm = scene.world;

	if (cosm.get_total_steps_passed() <= clean_round_state.clk.now.step + 1) {
		/* Otherwise we'd have an infinite loop. */
		return false;
	}

	for (const auto& n : notifications) {
		if (const auto tp = std::get_if<messages::teleportation>(std::addressof(n.payload))) {
			if (tp->teleported == viewed_character_id) {
				if (const auto portal = find<portal_marker>(tp->to_portal)) {
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

		if (tutorial.level < 4) {
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

#if 0
	using namespace augs::gui::text;
	using namespace augs::gui;

	const auto screen_size = in.screen_size;

	using FS = formatted_string;

	auto colored = [&](const auto& text, const auto& color) {
		return FS(std::string(text), { in.gui_fonts.gui, color });
	};

	print_stroked(
		in.get_drawer(),
		vec2i(screen_size.x / 2, 2),
		colored("Tutorial", white),
		{ augs::ralign::CX, augs::ralign::T }
	);
#endif
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