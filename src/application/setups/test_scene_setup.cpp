#include <cstdint>
#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_json_table.h"
#include "application/setups/test_scene_setup.h"

#include "application/setups/editor/packaged_official_content.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/arena/build_arena_from_editor_project.hpp"

#include "game/messages/game_notification.h"
#include "application/setups/editor/project/editor_project.hpp"
#include "augs/string/typesafe_sscanf.h"
#include "augs/gui/text/printer.h"

#include "game/modes/detail/delete_with_held_items.hpp"
#include "game/detail/hand_fuse_logic.h"
#include "game/detail/calc_ammo_info.hpp"

#include "augs/misc/web_sdk_events.h"

void web_sdk_happy_time();
void snap_interpolated_to_logical(cosmos& cosm);

using portal_marker = editor_area_marker_node;

test_scene_setup::test_scene_setup(
	std::string nickname,
	std::vector<std::byte> avatar_bytes,
	const packaged_official_content& official,
	//const input_recording_type recording_type,
	const test_scene_type type
	) : official(official), nickname(nickname), type(type), avatar_bytes(avatar_bytes) {
	init(type);
}

void test_scene_setup::init(const test_scene_type new_type) {
	auto loading_raii = web_sdk_loading_raii();

	type = new_type;

	if (type == test_scene_type::TUTORIAL) {
		current_arena_folder = "content/menu/tutorial";
	}
	else {
		current_arena_folder = "content/menu/shooting_range";
	}

	{
		auto json_read_settings = editor_project_readwrite::reading_settings();
		json_read_settings.read_inactive_nodes = false;

		project = editor_project_readwrite::read_project_json(
			get_paths().project_json,
			official.resources,
			official.resource_map,
			json_read_settings,
			nullptr
		);
	}

	name_to_node = project.make_name_to_node_map();

	{
		auto& markers = project.nodes.template get_pool_for<editor_point_marker_node>();

		bool found = false;

		for (auto& p : markers) {
			if (p.editable.faction == faction_type::METROPOLIS) {
				if (!p.active) {
					continue;
				}

				if (found) {
					p.active = false;
					continue;
				}

				found = true;
				const auto id = editor_typed_node_id<editor_point_marker_node>::from_raw(markers.get_id_of(p)).operator editor_node_id();

				const auto parent = project.find_parent_layer(id);

				ensure(parent.has_value());
				ensure(parent->layer_ptr != nullptr);

				if (!typesafe_sscanf(parent->layer_ptr->unique_name, "Level%x", tutorial.level)) {
					tutorial.level = 0;
				}
			}
		}
	};

	for (auto& l : project.layers.pool) {
		if (begins_with(l.unique_name, "Level")) {
			uint32_t level = 0;

			if (1 == typesafe_sscanf(l.unique_name, "Level%x", level)) {
				max_tutorial_level = std::max(level, max_tutorial_level);
			}
		}
	}

	LOG("Tutorial levels: %x", max_tutorial_level);

	restart_arena();

	if (!is_tutorial()) {
		if (const auto portal = find<portal_marker>("portal")) {
			range_entry_portal = portal->scene_entity_id;
		}
	}

	auto& cosm = scene.world;
	snap_interpolated_to_logical(cosm);
	//if (recording_type != input_recording_type::DISABLED) {
		//if (player.try_to_load_or_save_new_session(USER_DIR / "sessions/", "recorded.inputs")) {
		//
		//}
	//}

	/* Close any window that might be open by default */
	escape();
}

void test_scene_setup::set_tutorial_surfing_challenge() {
	set_tutorial_level(max_tutorial_level);
}

std::string test_scene_setup::get_browser_location() const {
	if (is_tutorial()) {
		std::string out = "tutorial";

		if (tutorial.level == 0) {
			return out;
		}

		if (tutorial.challenge) {
			out += "/challenge";
		}
		else {
			out += "/";
			out += std::to_string(tutorial.level);
		}

		return out;
	}
	else {
		return "range";
	}
}

void test_scene_setup::set_tutorial_level(uint32_t level) {
	auto& cosm = scene.world;

	auto character = [&]() {
		return cosm[get_controlled_character_id()];
	};

	tutorial.level = std::min(max_tutorial_level, level);

	const bool is_challenge = tutorial.level == max_tutorial_level;

	if (is_challenge) {
		tutorial.level = max_tutorial_level - 1;
		tutorial.challenge = true;
	}

	restart_arena();
	
	if (is_challenge) {
		if (const auto tp = find<portal_marker>("hard_surf_start")) {
			character().set_logic_transform(tp->editable.pos);
		}
	}

	snap_interpolated_to_logical(cosm);
}

void test_scene_setup::restart_arena() {
	LOG("Setting up tutorial level: %x.", tutorial.level);

	range_entry_portal = {};

	if (const bool completed = tutorial.level == 20) {
		web_sdk_happy_time();
	}

	auto& cosm = scene.world;

	auto character = [&]() {
		return cosm[get_controlled_character_id()];
	};

	auto pre_crosshair = character() ? character().get<components::crosshair>() : components::crosshair();
	auto pre_movement_flags = character() ? character().get<components::movement>().flags : components::movement().flags;

	auto get_next = [&](auto l) -> uint32_t {
		if (l == 5) {
			// skip deagles level (6)
			return 7;
		}

		return l + 1;
	};

	{
		for (auto& l : project.layers.pool) {
			if (begins_with(l.unique_name, "Level")) {
				uint32_t layer_level = 0;

				if (!typesafe_sscanf(l.unique_name, "Level%x", layer_level)) {
					l.editable.active = false;
				}

				l.editable.active = layer_level == tutorial.level || layer_level == get_next(tutorial.level);

				if (tutorial.level == 9) {
					if (layer_level == 11) {
						/* We need this for the mouse wheel skip portal */
						l.editable.active = true;
					}
				}
			}
		}

		project.clear_cached_scene_node_data();
		opponents.clear();

		::build_arena_from_editor_project(
			get_arena_handle(),
			{
				project,
				"",
				get_paths().project_folder,
				official,
				std::addressof(entity_to_node),
				nullptr,
				false, /* for_playtesting */
				false /* editor_preview */
			}
		);

		clean_step_number = scene.world.get_clock().now.step;
	}

	restart_mode();

	if (is_tutorial()) {
		if (tutorial.level == 0) { 
			/* First level has a nice default. */
		}
		else {
			character().get<components::crosshair>() = pre_crosshair;
		}
	}

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

template <class T>
T* test_scene_setup::find(const entity_id& id) {
	return find<T>(::entity_to_node_id(entity_to_node, id));
}

template <class T>
T* test_scene_setup::find(const editor_node_id& generic_id) {
	return project.find_node<T>(generic_id);
}

template <class T>
T* test_scene_setup::find(const std::string& name) {
	if (auto generic_id = mapped_or_nullptr(name_to_node, name)) {
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
			::delete_with_held_items_except({}, h);
		}
		else {
			step.queue_deletion_of(h, "test_scene_setup::remove");
		}
	}
}

void test_scene_setup::restart_mode() {
	should_init_level = true;
	restart_arena_in_ms = -1;

	auto& cosm = scene.world;

	const auto current_teleport = typesafe_sprintf("entry%x", tutorial.level);

	const bool is_akimbo_level = tutorial.level == 5;
	const bool is_duals_level = tutorial.level == 6;
	const bool is_ricochets_level = tutorial.level == 7;
	const bool is_try_throwing_reloading_level = tutorial.level == 10 || tutorial.level == 13;
	const bool is_planting_level = tutorial.level == 14;
	const bool is_defusing_level = tutorial.level == 15;
	const bool is_normal_spells_level = tutorial.level == 16;
	const bool is_offensive_spells_level = tutorial.level == 17;

	const auto player_faction = is_planting_level ? faction_type::RESISTANCE : faction_type::METROPOLIS;
	const auto enemy_faction  = player_faction == faction_type::METROPOLIS ? faction_type::RESISTANCE : faction_type::METROPOLIS;

	if (is_planting_level) {
		auto b1 = to_handle("bomb1");
		auto b2 = to_handle("bomb2");

		b1.template get<components::hand_fuse>().fuse_delay_ms = 3000;
		b2.template get<components::hand_fuse>().fuse_delay_ms = 2000;
	}

	if (is_defusing_level) {
		auto b1 = to_handle("planted1");
		auto b2 = to_handle("planted2");

		b1.template get<components::hand_fuse>().fuse_delay_ms = 20000;
		b2.template get<components::hand_fuse>().fuse_delay_ms = 18000;
	}

	get_arena_handle().on_mode_with_input(
		[&]<typename M>(M& mode, const auto& input) {
			if constexpr(std::is_same_v<test_mode, M>) {
				local_player_id = mode.add_player(input, nickname, player_faction);
				viewed_character_id = cosm[mode.lookup(local_player_id)].get_id();

				for (auto& p : project.nodes.template get_pool_for<editor_point_marker_node>()) {
					if (p.scene_entity_id.is_set() && p.editable.faction == faction_type::RESISTANCE) {
						const auto new_id = mode.add_player(input, nickname, enemy_faction);
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

				const auto new_id = local_player_id;
				auto player = mode.find(new_id);

				ensure(player != nullptr)

				if (const auto tp = find<portal_marker>(current_teleport)) {
					player->dedicated_spawn = tp->scene_entity_id;
					mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
				}

				if (!is_tutorial() || is_akimbo_level || is_duals_level || is_ricochets_level || is_try_throwing_reloading_level || is_defusing_level) {
					mode.infinite_ammo_for = viewed_character_id;
				}

				if (is_normal_spells_level) {
					auto& s = cosm[viewed_character_id].get<components::sentience>();

					s.learnt_spells[spell_id::of<haste>().get_index()] = true;
					s.learnt_spells[spell_id::of<exaltation>().get_index()] = true;
					s.learnt_spells[spell_id::of<echoes_of_the_higher_realms>().get_index()] = true;
				}

				if (is_offensive_spells_level) {
					auto& s = cosm[viewed_character_id].get<components::sentience>();

					s.spells_drain_pe = false;

					s.learnt_spells[spell_id::of<fury_of_the_aeons>().get_index()] = true;
					s.learnt_spells[spell_id::of<ultimate_wrath_of_the_aeons>().get_index()] = true;
					s.learnt_spells[spell_id::of<electric_triad>().get_index()] = true;
				}

				if (const bool is_range = !is_tutorial()) {
					auto& s = cosm[viewed_character_id].get<components::sentience>();

					s.spells_drain_pe = false;
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

	auto& cosm = scene.world;

	if (const bool is_planting_level = tutorial.level == 14) {
		if (auto armor = cosm[viewed_character_id][slot_function::TORSO_ARMOR].get_item_if_any()) {
			step.queue_deletion_of(armor, "disable armor");
		}

		if (to_handle("bomb1").dead()) {
			remove(step, "obs_bomb1");
		}

		if (to_handle("bomb2").dead()) {
			remove(step, "obs_bomb2");
		}
	}

	if (restart_arena_in_ms > 0) {
		restart_arena_in_ms -= step.get_delta().in_milliseconds();

		if (restart_arena_in_ms <= 0) {
			restart_arena();
		}
	}

	if (const bool is_weapon_level = tutorial.level == 4) {
		if (const bool still_first_stage = to_handle("obs1").alive()) {
			const bool baka_empty = 0 == ::calc_ammo_info(to_handle("baka47")).total_charges;

			if (baka_empty) {
				if (restart_arena_in_ms < 0) {
					restart_arena_in_ms = 500.0f;
				}
			}
		}
	}

	if (const bool is_defusing_level = tutorial.level == 15) {
		auto carrier = to_handle("carrier");
		auto carrier_bomb = to_handle("carrier_bomb");

		ensure(carrier); 
		ensure(carrier_bomb);

		if (should_init_level) {
			const auto pickup_slot = carrier.find_pickup_target_slot_for(carrier_bomb, { slot_finding_opt::OMIT_MOUNTED_SLOTS });

			if (pickup_slot.alive()) {
				perform_transfer(item_slot_transfer_request::standard(entity_id(carrier_bomb.get_id()), pickup_slot), step);
			}
		}
		else {
			if (carrier_bomb.get_owning_transfer_capability() == viewed_character_id) {
				remove(step, "obs_carrier");
			}
		}

		auto b1 = to_handle("planted1");
		auto b2 = to_handle("planted2");

		if (b1.dead() || b2.dead()) {
			if (restart_arena_in_ms < 0) {
				restart_arena_in_ms = 1000.0f;
			}
		}
		else {
			bool arm_second = false;

			b1.dispatch_on_having_all<components::hand_fuse>([&](const auto& typed_fused) {
				auto fuse_logic = fuse_logic_provider(typed_fused, step);

				if (should_init_level) {
					fuse_logic.arm_explosive(arming_source_type::SHOOT_INTENT, false);
				}
				else {
					if (fuse_logic.defused()) {
						arm_second = true;
						remove(step, "obs_def1");
					}
				}
			});

			b2.dispatch_on_having_all<components::hand_fuse>([&](const auto& typed_fused) {
				auto fuse_logic = fuse_logic_provider(typed_fused, step);

				if (fuse_logic.defused()) {
					remove(step, "obs_def2");
				}
				else {
					if (arm_second && !fuse_logic.armed()) {
						fuse_logic.arm_explosive(arming_source_type::SHOOT_INTENT, true);
					}
				}
			});
		}
	}

	for (int i = 1; i < 30; ++i) {
		if (!exists(i, 0)) {
			continue;
		}

		bool all_killed = true;

		int j = 0;

		while (exists(i, j)) {
			if (!killed(i, j)) {
				if (const auto disable_armor = i == 4 || i == 14 || i == 16 || i == 18 || i == 19 || i == 22 || i == 23 || i == 24) {
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
				get_arena_handle().on_mode_with_input(
					[&]<typename M>(M& mode, const auto&) {
						if constexpr(std::is_same_v<test_mode, M>) {
							mode.for_each_player_in(faction_type::RESISTANCE, [&mode](auto id, auto) { mode.find(id)->allow_respawn = false; });
						}
					}
				);
			}

			int k = 0;

			while (to_entity(obs(i, k)).is_set()) {
				remove(step, obs(i, k));
				++k;
			}
		}
	}

	should_init_level = false;
}

void test_scene_setup::pre_solve(const logic_step step) {
	if (const auto h = scene.world[viewed_character_id]) {
		cosmic::set_specific_name(h, nickname);

		get_arena_handle().on_mode_with_input(
			[&]<typename M>(M& mode, const auto&) {
				if constexpr(std::is_same_v<test_mode, M>) {
					mode.players[local_player_id].session.nickname = nickname;
				}
			}
		);
	}

	do_tutorial_logic(step);
}

bool test_scene_setup::post_solve(const const_logic_step step) {
	const auto& notifications = step.get_queue<messages::game_notification>();

	auto& cosm = scene.world;

	if (range_entry_portal.is_set()) {
		snap_interpolated_to_logical(scene.world);

		for (auto& s : step.get_queue<messages::start_sound_effect>()) {
			if (const auto portal = cosm[range_entry_portal]) {
				if (s.payload.input.id == portal.get<components::portal>().exit_sound.id) {
					s.payload.input.modifier.always_direct_listener = true;
				}
			}
		}
	}

#if PLATFORM_WEB
	const auto steps_until_start_watching_player = 4;
#else
	const auto steps_until_start_watching_player = 1;
#endif

	if (cosm.get_total_steps_passed() >= steps_until_start_watching_player) {
		range_entry_portal = {};
	}

	if (restart_requested) {
		restart_requested = false;

		restart_arena();
		return true;
	}

	if (cosm.get_total_steps_passed() <= clean_step_number + 1) {
		/* Otherwise we'd have an infinite loop. */
		return false;
	}

	bool shooting_range = false;

	for (const auto& n : notifications) {
		if (const auto tp = std::get_if<messages::teleportation>(std::addressof(n.payload))) {
			if (tp->teleported == viewed_character_id) {
				if (const auto portal = find<portal_marker>(tp->to_portal)) {
					const auto& name = portal->unique_name;

					if (name == "main_menu") {
						special_result = custom_imgui_result::GO_TO_MAIN_MENU;
					}

					if (name == "shooting_range") {
						shooting_range = true;
					}

					if (begins_with(name, "hard_surf_start")) {
						tutorial.challenge = true;
					}

					if (begins_with(name, "entry")) {
						tutorial.challenge = false;
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

	if (shooting_range) {
		auto character = [&]() {
			return cosm[get_controlled_character_id()];
		};

		auto pre_crosshair = character() ? character().get<components::crosshair>() : components::crosshair();
		auto pre_movement_flags = character() ? character().get<components::movement>().flags : components::movement().flags;

		init(test_scene_type::SHOOTING_RANGE);

		if (const auto ch = character()) {
			if (const auto cr = ch.find<components::crosshair>()) {
				*cr = pre_crosshair;
			}

			if (const auto mv = ch.find<components::movement>()) {
				mv->flags = pre_movement_flags;
			}
		}
	}

	return false;
}

std::string test_scene_setup::get_scoreboard_caption() const {
	if (is_tutorial()) {
		return typesafe_sprintf("Tutorial: Level %x of %x", tutorial.level, max_tutorial_level);
	}
	else {
		return "Shooting Range";
	}
}

void test_scene_setup::customize_for_viewing(config_json_table& config) const {
	if (is_tutorial()) {
		config.window.name = "Hypersomnia - Tutorial";
	}
	else {
		config.window.name = "Hypersomnia - Shooting range";
	}

	if (speed < 1.0f) {
		//config.interpolation.method = interpolation_method::NONE;
	}

	nickname = config.client.get_nickname();

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

void test_scene_setup::get_steam_rich_presence_pairs(steam_rich_presence_pairs& pairs) const {
	if (is_tutorial()) {
		pairs.push_back({ "steam_display", "#Status_Tutorial" });
		pairs.push_back({ "level", std::to_string(tutorial.level) });
	}
	else {
		pairs.push_back({ "steam_display", "#Status_ShootingRange" });
	}
}

void test_scene_setup::set_new_avatar(std::vector<std::byte> bytes) {
	avatar_bytes = std::move(bytes);
	rebuild_player_meta_viewables = true;
}

std::optional<arena_player_metas> test_scene_setup::get_new_player_metas() {
	if (rebuild_player_meta_viewables) {
		auto& metas = player_metas;
		
		metas[mode_player_id::first().value].avatar.image_bytes = avatar_bytes;

		rebuild_player_meta_viewables = false;
		return metas;
	}

	return std::nullopt;
}