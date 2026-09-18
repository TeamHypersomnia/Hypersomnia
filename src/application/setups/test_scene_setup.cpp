#include <cstdint>
#include <sstream>
#include <iomanip>
#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_json_table.h"
#include "application/setups/test_scene_setup.h"

#include "application/setups/editor/packaged_official_content.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/arena/build_arena_from_editor_project.hpp"

#include "game/messages/game_notification.h"
#include "game/components/sprite_component.h"
#include "game/components/portal_component.h"
#include "game/detail/physics/shape_overlapping.hpp"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"
#include "application/setups/editor/project/editor_project.hpp"
#include "augs/string/typesafe_sscanf.h"
#include "augs/gui/text/printer.h"
#include "application/setups/draw_setup_gui_input.h"
#include "view/rendering_scripts/minimap_layout.h"
#include "view/mode_gui/arena/on_first_touching_portal.hpp"

#include "game/modes/detail/delete_with_held_items.hpp"
#include "game/detail/hand_fuse_logic.h"
#include "game/detail/calc_ammo_info.hpp"

#include "augs/misc/web_sdk_events.h"
#include "augs/templates/algorithm_templates.h"

#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/test_scene_flavour_ids.h"

void web_sdk_happy_time();
void snap_interpolated_to_logical(cosmos& cosm);

static std::optional<item_flavour_id> test_enemy_weapon;//= to_entity_flavour_id(test_shootable_weapons::BAKA47);

using portal_marker = editor_area_marker_node;

/*
	Testing: when 1, the tutorial starts right on the finish screen
	(the one congratulating on completing the basic tutorial),
	so the advanced tutorial can be tested right away.
*/
#define TEST_START_AT_TUTORIAL_FINISH_SCREEN 0

/*
	Testing: when 1, the shooting range draws a test exp bar at the bottom,
	filled by the character's horizontal movement - to interactively test
	the HUD bar rendering.
*/
#define TEST_EXP_BAR_IN_RANGE 0

/*
	Level0..Level4 are the basic tutorial.
	The rest of the stage structure (the finish screen, the advanced levels)
	is derived from the portal graph in init().
*/

constexpr uint32_t tutorial_last_basic_level_v = 4;

/* Based on the height of the character's value bars (their 16px icons). */
constexpr int tutorial_hud_bar_h_v = 16;

/* Both progress bars are slightly taller. */
constexpr int tutorial_bottom_bar_h_v = tutorial_hud_bar_h_v + 5;
constexpr int tutorial_stage_bar_h_v = tutorial_hud_bar_h_v + 1;

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

	dummy_dynamic_vars.friendly_fire = true;

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

	/*
		Derive the tutorial's stage structure factually from the portal graph,
		instead of trusting the numbers in the layer names -
		some levels are excluded from circulation
		(either their layers are inactive - these are skipped at read time
		altogether - or no portal ever leads to them).

		Level0..Level4 are the basic tutorial.
		The level that the last basic level exits to is the finish screen -
		no progress HUD is drawn there.
		Everything reachable from the finish screen onwards is the advanced tutorial.
	*/

	basic_tutorial_levels.clear();
	advanced_tutorial_levels.clear();
	tutorial_finish_level = std::nullopt;
	current_tip_portals.clear();
	visited_tip_portals.clear();

	{
		std::unordered_map<uint32_t, std::vector<uint32_t>> level_edges;

		for (auto& l : project.layers.pool) {
			if (!begins_with(l.unique_name, "Level")) {
				continue;
			}

			uint32_t source_level = 0;

			if (1 != typesafe_sscanf(l.unique_name, "Level%x", source_level)) {
				continue;
			}

			max_tutorial_level = std::max(source_level, max_tutorial_level);

			if (source_level <= tutorial_last_basic_level_v) {
				basic_tutorial_levels.push_back(source_level);
			}

			for (const auto& node_id : l.hierarchy.nodes) {
				if (const auto portal = find<portal_marker>(node_id)) {
					const auto exit_id = portal->editable.as_portal.portal_exit.operator editor_node_id();

					if (const auto exit_layer = project.find_parent_layer(exit_id)) {
						if (exit_layer->layer_ptr != nullptr) {
							uint32_t target_level = 0;

							if (1 == typesafe_sscanf(exit_layer->layer_ptr->unique_name, "Level%x", target_level)) {
								if (target_level != source_level) {
									level_edges[source_level].push_back(target_level);
								}
							}
						}
					}
				}
			}
		}

		sort_range(basic_tutorial_levels);

		if (!basic_tutorial_levels.empty()) {
			for (const auto target : level_edges[basic_tutorial_levels.back()]) {
				if (target > tutorial_last_basic_level_v) {
					tutorial_finish_level = target;
					break;
				}
			}
		}

		if (tutorial_finish_level.has_value()) {
			std::unordered_set<uint32_t> reached = { *tutorial_finish_level };
			std::vector<uint32_t> queue = { *tutorial_finish_level };

			while (!queue.empty()) {
				const auto current = queue.back();
				queue.pop_back();

				for (const auto target : level_edges[current]) {
					if (target > tutorial_last_basic_level_v && reached.emplace(target).second) {
						advanced_tutorial_levels.push_back(target);
						queue.push_back(target);
					}
				}
			}

			sort_range(advanced_tutorial_levels);
		}
	}

	LOG("Tutorial levels: %x", max_tutorial_level);
	LOG("Tutorial basic stages: %x, advanced stages: %x, finish level: %x", basic_tutorial_levels.size(), advanced_tutorial_levels.size(), tutorial_finish_level ? static_cast<int>(*tutorial_finish_level) : -1);

#if TEST_START_AT_TUTORIAL_FINISH_SCREEN
	if (is_tutorial() && tutorial_finish_level.has_value()) {
		tutorial.level = *tutorial_finish_level;
	}
#endif

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

	{
		/*
			Activate the current tutorial level's layer, plus every layer that
			the current level's outgoing portals lead to (the layer owning each
			portal's portal_exit). This way the outgoing layers always exist
			without hardcoding the relations between levels.
		*/

		for (auto& l : project.layers.pool) {
			if (begins_with(l.unique_name, "Level")) {
				l.editable.active = false;
			}
		}

		const auto current_level_name = "Level" + std::to_string(tutorial.level);

		if (const auto current_layer = project.find_layer(current_level_name)) {
			current_layer->editable.active = true;

			for (const auto& node_id : current_layer->hierarchy.nodes) {
				if (const auto portal = find<portal_marker>(node_id)) {
					const auto exit_id = portal->editable.as_portal.portal_exit.operator editor_node_id();

					if (const auto exit_layer = project.find_parent_layer(exit_id)) {
						if (const auto exit_layer_ptr = project.find_layer(exit_layer->layer_id)) {
							exit_layer_ptr->editable.active = true;
						}
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

	refresh_range_zoom_pads();
	refresh_faction_pads();
	refresh_tip_portals();
}

void test_scene_setup::refresh_tip_portals() {
	current_tip_portals.clear();

	/*
		Any restart of the arena - a death, a checkpoint restart
		from the menu, or a level change - resets the progress.
	*/

	visited_tip_portals.clear();

	/*
		Cancel any pending flash: its timestamp comes from the previous
		cosmos' clock, and the rebuilt cosmos starts counting from zero again,
		so a stale flash would replay on the fresh bar.
		last_ratio is kept, so a genuine increase across a level change still flashes.
	*/

	bottom_bar_highlight.flashes.clear();
	stage_bar_highlight.flashes.clear();

	/*
		On the finish screen, zero the trackers altogether,
		so the advanced tutorial starts fresh and its first
		progress gains highlight correctly.
	*/

	if (is_tutorial_finish_level()) {
		bottom_bar_highlight = {};
		stage_bar_highlight = {};
	}

	if (!is_tutorial()) {
		return;
	}

	const auto current_level_name = "Level" + std::to_string(tutorial.level);

	if (const auto current_layer = project.find_layer(current_level_name)) {
		for (const auto& node_id : current_layer->hierarchy.nodes) {
			if (const auto portal = find<portal_marker>(node_id)) {
				const auto& info = portal->editable.as_portal;

				const bool is_green_tip_portal =
					info.color_preset == editor_color_preset::GREEN
					&& info.context_tip.is_enabled
					&& !info.context_tip.value.empty()
				;

				if (is_green_tip_portal) {
					current_tip_portals.emplace(portal->unique_name);
				}
			}
		}
	}
}

bool test_scene_setup::is_tutorial_finish_level() const {
	return is_tutorial() && tutorial_finish_level.has_value() && tutorial.level == *tutorial_finish_level;
}

bool test_scene_setup::should_draw_bottom_progress_bar() const {
	return is_tutorial() && !is_tutorial_finish_level() && !current_tip_portals.empty();
}

bool test_scene_setup::should_draw_stage_bar() const {
	return is_tutorial() && get_tutorial_stage_num_and_count().has_value();
}

std::optional<std::pair<uint32_t, uint32_t>> test_scene_setup::get_tutorial_stage_num_and_count() const {
	if (!is_tutorial()) {
		return std::nullopt;
	}

	if (is_tutorial_finish_level()) {
		return std::nullopt;
	}

	const auto stage_among = [&](const std::vector<uint32_t>& levels) -> std::optional<std::pair<uint32_t, uint32_t>> {
		uint32_t stage = 0;
		bool found = false;

		for (const auto level : levels) {
			if (level <= tutorial.level) {
				++stage;
			}

			if (level == tutorial.level) {
				found = true;
			}
		}

		if (!found) {
			return std::nullopt;
		}

		return std::pair { stage, static_cast<uint32_t>(levels.size()) };
	};

	if (tutorial.level <= tutorial_last_basic_level_v) {
		return stage_among(basic_tutorial_levels);
	}

	return stage_among(advanced_tutorial_levels);
}

void test_scene_setup::refresh_range_zoom_pads() {
	range_zoom_pads.clear();

	if (is_tutorial()) {
		return;
	}

	/*
		Scans for all node pairs named "zoom_<value>x" (the magnifier icon)
		and "zoom_<value>x_outline" (the pad's outline),
		so new pads can be added in the editor without touching the code.
	*/

	for (const auto& entry : name_to_node) {
		const auto& name = entry.first;

		if (!begins_with(name, "zoom_") || !ends_with(name, "x")) {
			continue;
		}

		auto zoom = 0.0f;

		if (1 != typesafe_sscanf(name, "zoom_%xx", zoom) || zoom <= 0.0f) {
			continue;
		}

		auto pad = range_zoom_pad();
		pad.zoom = zoom;

		if (const auto icon = find<editor_sprite_node>(name)) {
			pad.icon = icon->scene_entity_id;

			if (const auto icon_handle = scene.world[pad.icon]) {
				icon_handle.dispatch_on_having_all<components::sprite>(
					[&](const auto& typed_icon) {
						pad.inactive_neon = typed_icon.template get<components::sprite>().colorize_neon;
						pad.inactive_neon.a = 100;
					}
				);
			}
		}

		if (const auto outline = find<editor_sprite_node>(name + "_outline")) {
			pad.outline = outline->scene_entity_id;

			if (const auto outline_handle = scene.world[pad.outline]) {
				outline_handle.dispatch_on_having_all<components::sprite>(
					[&](const auto& typed_outline) {
						pad.inactive_outline = typed_outline.template get<components::sprite>().colorize;
					}
				);
			}
		}

		if (pad.outline.is_set() && pad.icon.is_set()) {
			range_zoom_pads.push_back(pad);
		}
	}

	/*
		The pads reuse the begin entering sound of the range's entry portal.
	*/

	if (const auto entry = find<portal_marker>("portal")) {
		if (const auto handle = scene.world[entry->scene_entity_id]) {
			if (const auto* const portal = handle.find<components::portal>()) {
				range_pad_sound = portal->begin_entering_sound;
			}
		}
	}

	/*
		The pad matching the map's default zoom starts out active.
	*/

	if (!range_zoom_pads.empty() && !range_zoom_override.has_value()) {
		range_zoom_override = project.settings.default_zoom;
	}
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

	auto player_faction = is_planting_level ? faction_type::RESISTANCE : faction_type::METROPOLIS;

	if (!is_tutorial()) {
		if (const auto tp = find<editor_point_marker_node>("range_spawn")) {
			player_faction = tp->editable.faction;
		}
	}

	auto enemy_faction  = player_faction == faction_type::METROPOLIS ? faction_type::RESISTANCE : faction_type::METROPOLIS;

	if (test_enemy_weapon) {
		if (auto* rs = std::get_if<test_mode_ruleset>(&ruleset)) {
			rs->factions[enemy_faction].round_start_eq.weapon = *test_enemy_weapon;
		}
	}


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
					if (!p.scene_entity_id.is_set()) {
						continue;
					}

					if (p.get_display_name() == "range_spawn") {
						continue;
					}

					if (is_tutorial()) {
						if (p.editable.faction == player_faction) {
							continue;
						}

						const auto new_id = mode.add_player(input, nickname, enemy_faction);
						mode.find(new_id)->dedicated_spawn = p.scene_entity_id;
						mode.find(new_id)->hide_in_scoreboard = true;
						const auto opponent_id = mode.find(new_id)->controlled_character_id;
						mode.teleport_to_next_spawn(input, new_id, opponent_id);
						mode.find(new_id)->allow_respawn = is_akimbo_level;
						opponents[p.unique_name] = opponent_id;
					}
					else {
						const auto new_id = mode.add_player(input, nickname, p.editable.faction);
						mode.find(new_id)->dedicated_spawn = p.scene_entity_id;
						mode.find(new_id)->hide_in_scoreboard = true;
						const auto opponent_id = mode.find(new_id)->controlled_character_id;
						mode.teleport_to_next_spawn(input, new_id, opponent_id);
						mode.find(new_id)->allow_respawn = false;
						opponents[p.unique_name] = opponent_id;
					}
				}

				const auto new_id = local_player_id;
				auto player = mode.find(new_id);

				ensure(player != nullptr)

				if (is_tutorial()) {
					if (const auto tp = find<portal_marker>(current_teleport)) {
						player->dedicated_spawn = tp->scene_entity_id;
						mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
					}
				}
				else {
					if (const auto tp = find<editor_point_marker_node>("range_spawn")) {
						player->dedicated_spawn = tp->scene_entity_id;
						mode.teleport_to_next_spawn(input, new_id, mode.find(new_id)->controlled_character_id);
					}
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
					fill_range(s.learnt_spells, true);
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

	{
		/*
			Track the visited context-tip portals for the bottom progress bar.

			Poll the same way the context tip drawing detects the tip
			for the character - this also catches the portal the character
			spawns in, whose entering the portal system never reports.
		*/

		::on_first_touching_portal(
			cosm[viewed_character_id],
			[&](const auto& touched_portal) {
				if (const auto portal = find<portal_marker>(entity_id(touched_portal.get_id()))) {
					if (found_in(current_tip_portals, portal->unique_name)) {
						visited_tip_portals.emplace(portal->unique_name);
					}
				}
			}
		);
	}

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
	do_range_zoom_pads_logic(step);
	do_faction_pads_logic(step);

#if TEST_EXP_BAR_IN_RANGE
	if (!is_tutorial()) {
		/*
			The test bar follows the character's horizontal velocity:
			10% per second of moving at 500 units/s - rightward movement
			adds, leftward subtracts.
		*/

		if (const auto character = scene.world[viewed_character_id]) {
			if (const auto rigid_body = character.find<components::rigid_body>()) {
				const auto velocity_x = rigid_body.get_velocity().x;

				range_test_bar_ratio += velocity_x / 500.0f * 0.10f * step.get_delta().in_seconds();
				range_test_bar_ratio = std::clamp(range_test_bar_ratio, 0.0f, 1.0f);
			}
		}
	}
#endif
}

/*
	The pads are detected against the character's full collider geometry,
	every step - portal contacts proved unreliable for the adjacent pads.

	A pad activates the moment the character's collider touches its outline.
	When two pads are touched at once, the one closer to the character's center wins.
*/

template <class P>
static const P* find_best_touched_pad(
	cosmos& cosm,
	const entity_handle character,
	const std::vector<P>& pads
) {
	const auto transform = character.find_logic_transform();

	if (!transform.has_value()) {
		return nullptr;
	}

	const auto si = cosm.get_si();

	const P* best_pad = nullptr;
	auto best_dist_sq = 0.0f;

	for (const auto& pad : pads) {
		const auto outline = cosm[pad.outline];

		if (outline.dead()) {
			continue;
		}

		const auto outline_transform = outline.find_logic_transform();

		if (!outline_transform.has_value()) {
			continue;
		}

		bool overlaps = false;

		outline.template dispatch_on_having_all<components::sprite>(
			[&](const auto& typed_outline) {
				const auto pad_size = typed_outline.get_logical_size();

				/*
					The activation box is shrunk so that the character
					has to visibly step onto the pad.
				*/
				const auto activation_margin = 10.0f;

				b2PolygonShape pad_box;

				pad_box.SetAsBox(
					si.get_meters(std::max(1.0f, pad_size.x / 2 - activation_margin)),
					si.get_meters(std::max(1.0f, pad_size.y / 2 - activation_margin))
				);

				/*
					Exact test against the character's true physical hitbox
					(the convex hull derived from the sprite),
					skipping any sensor fixtures.
				*/

				::for_each_fixture(character, [&](const b2Fixture& fixture) -> std::optional<bool> {
					if (fixture.IsSensor()) {
						return std::nullopt;
					}

					if (::shape_overlaps_fixture(std::addressof(pad_box), si, *outline_transform, fixture).has_value()) {
						overlaps = true;
						return true;
					}

					return std::nullopt;
				});
			}
		);

		if (!overlaps) {
			continue;
		}

		const auto dist_sq = (outline_transform->pos - transform->pos).length_sq();

		if (best_pad == nullptr || dist_sq < best_dist_sq) {
			best_pad = std::addressof(pad);
			best_dist_sq = dist_sq;
		}
	}

	return best_pad;
}

void test_scene_setup::do_range_zoom_pads_logic(const logic_step step) {
	if (range_zoom_pads.empty()) {
		return;
	}

	auto& cosm = scene.world;
	const auto character = cosm[viewed_character_id];

	if (character.dead()) {
		return;
	}

	{
		const auto best_pad = ::find_best_touched_pad(cosm, character, range_zoom_pads);

		if (best_pad != nullptr) {
			const bool changed = !range_zoom_override.has_value() || *range_zoom_override != best_pad->zoom;

			if (changed) {
				range_zoom_override = best_pad->zoom;

				auto effect = range_pad_sound;

				effect.start(
					step,
					sound_effect_start_input::at_listener(character),
					always_predictable_v
				);
			}
		}
	}

	for (const auto& pad : range_zoom_pads) {
		const bool active =
			range_zoom_override.has_value()
			&& *range_zoom_override == pad.zoom
		;

		if (const auto icon = cosm[pad.icon]) {
			icon.dispatch_on_having_all<components::sprite>(
				[&](const auto& typed_icon) {
					typed_icon.template get<components::sprite>().colorize_neon = active ? rgba(0, 255, 0, 255) : pad.inactive_neon;
				}
			);
		}

		if (const auto outline = cosm[pad.outline]) {
			outline.dispatch_on_having_all<components::sprite>(
				[&](const auto& typed_outline) {
					typed_outline.template get<components::sprite>().colorize = active ? rgba(0, 255, 0, 255) : pad.inactive_outline;
				}
			);
		}
	}
}

void test_scene_setup::refresh_faction_pads() {
	faction_pads.clear();

	if (is_tutorial()) {
		return;
	}

	/*
		Scans for node pairs named "faction_<lowercase faction name>" (the faction logo)
		and "faction_<lowercase faction name>_outline" (the pad's outline).
		Stepping on such a pad recreates the character as the given faction,
		moving the whole equipment onto the new character.
	*/

	auto add_pad = [&](const std::string& name, const faction_type faction) {
		auto pad = faction_pad();
		pad.faction = faction;

		if (const auto icon = find<editor_sprite_node>(name)) {
			pad.icon = icon->scene_entity_id;

			if (const auto icon_handle = scene.world[pad.icon]) {
				icon_handle.dispatch_on_having_all<components::sprite>(
					[&](const auto& typed_icon) {
						pad.inactive_neon = typed_icon.template get<components::sprite>().colorize_neon;
						pad.inactive_neon.a = 100;
					}
				);
			}
		}

		if (const auto outline = find<editor_sprite_node>(name + "_outline")) {
			pad.outline = outline->scene_entity_id;

			if (const auto outline_handle = scene.world[pad.outline]) {
				outline_handle.dispatch_on_having_all<components::sprite>(
					[&](const auto& typed_outline) {
						pad.inactive_outline = typed_outline.template get<components::sprite>().colorize;
					}
				);
			}
		}

		if (pad.outline.is_set() && pad.icon.is_set()) {
			faction_pads.push_back(pad);
		}
	};

	add_pad("faction_metropolis", faction_type::METROPOLIS);
	add_pad("faction_resistance", faction_type::RESISTANCE);
}

void test_scene_setup::do_faction_pads_logic(const logic_step step) {
	if (faction_pads.empty()) {
		return;
	}

	auto& cosm = scene.world;
	const auto character = cosm[viewed_character_id];

	if (character.dead()) {
		return;
	}

	if (const auto best_pad = ::find_best_touched_pad(cosm, character, faction_pads)) {
		if (best_pad->faction != character.get_official_faction()) {
			const auto pad_faction = best_pad->faction;

			get_arena_handle().on_mode_with_input(
				[&]<typename M>(M& mode, const auto& input) {
					if constexpr(std::is_same_v<test_mode, M>) {
						const auto new_id = mode.change_player_faction(input, step, local_player_id, pad_faction);

						if (const auto new_character = cosm[new_id]; new_character && new_id != viewed_character_id) {
							viewed_character_id = new_id;

							auto effect = range_pad_sound;

							effect.start(
								step,
								sound_effect_start_input::at_listener(new_character),
								always_predictable_v
							);
						}
					}
				}
			);
		}
	}

	const auto viewed_faction = [&]() {
		if (const auto handle = cosm[viewed_character_id]) {
			return handle.get_official_faction();
		}

		return faction_type::SPECTATOR;
	}();

	for (const auto& pad : faction_pads) {
		const bool active = pad.faction == viewed_faction;

		if (const auto icon = cosm[pad.icon]) {
			icon.dispatch_on_having_all<components::sprite>(
				[&](const auto& typed_icon) {
					typed_icon.template get<components::sprite>().colorize_neon = active ? rgba(0, 255, 0, 255) : pad.inactive_neon;
				}
			);
		}

		if (const auto outline = cosm[pad.outline]) {
			outline.dispatch_on_having_all<components::sprite>(
				[&](const auto& typed_outline) {
					typed_outline.template get<components::sprite>().colorize = active ? rgba(0, 255, 0, 255) : pad.inactive_outline;
				}
			);
		}
	}
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

					if (begins_with(name, "main_menu")) {
						special_result = custom_imgui_result::GO_TO_MAIN_MENU;
					}

					if (begins_with(name, "shooting_range")) {
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

		/*
			Make room for the tutorial progress HUD:
			the minimap has to sit above the bottom progress bar,
			and anything sharing the minimap's corner (e.g. the value bars)
			has to make room for the stage bar block drawn at the minimap's edge.
		*/

		auto& minimap = config.drawing.minimap;

		if (should_draw_bottom_progress_bar()) {
			minimap.extra_bottom_margin = tutorial_bottom_bar_h_v;
		}

		if (should_draw_stage_bar() && minimap.occupies_corner(minimap.position)) {
			minimap.extra_hud_space = stage_block_height;
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

	draw_tutorial_hud(in);
	draw_range_test_bar(in);
}

/*
	The shared setup of the "aura" (exp-like) progress bars:
	the tutorial's bottom bar and the range's test bar.
*/

static hud_bar_appearance make_aura_bar_appearance() {
	auto appearance = hud_bar_appearance();

	/* A less garish green than the pure rgba one. */
	appearance.color = rgba(41, 130, 2, 255);
	appearance.border_w = 2;
	appearance.particle_tint = 0.12f;
	appearance.multiple_flashes = true;
	appearance.split_gap = 2;
	appearance.label_background = true;
	appearance.label_align_bottom = true;
	appearance.label_border_w = 6;
	appearance.label_unfilled_border_w = 2;
	appearance.label_padding = vec2i(20, 12);

	/* The dark inner outline drawn whole - in the unfilled part too. */
	appearance.label_inner_outline_shows_value = false;

	/* An opaque, solid backdrop under the label's text. */
	appearance.label_background_color = rgba(25, 56, 9, 255);

	return appearance;
}

/*
	Stretched over the whole screen's width,
	touching the bottom edge exactly - no padding.
*/

static ltrb make_aura_bar_rect(const vec2i screen_size) {
	return ltrb(
		0.0f,
		static_cast<float>(screen_size.y - tutorial_bottom_bar_h_v),
		static_cast<float>(screen_size.x),
		static_cast<float>(screen_size.y)
	);
}

static std::string make_aura_bar_label(
	const game_drawing_settings& drawing,
	const float ratio,
	const int current_value,
	const int max_value
) {
	if (!drawing.draw_value_on_aura_bar) {
		return {};
	}

	if (drawing.aura_bar_value_as_percent) {
		const auto decimal_places = std::clamp(drawing.aura_bar_percent_decimal_places, 0, 3);

		auto percent_stream = std::ostringstream();
		percent_stream << std::fixed << std::setprecision(decimal_places) << ratio * 100.0f << "%";

		return percent_stream.str();
	}

	return typesafe_sprintf("%x/%x", current_value, max_value);
}

void test_scene_setup::draw_range_test_bar(const draw_setup_gui_input& in) {
#if TEST_EXP_BAR_IN_RANGE
	if (is_tutorial()) {
		return;
	}

	const auto& cosm = scene.world;
	const auto total_secs = cosm.get_total_seconds_passed(get_interpolation_ratio());

	auto appearance = ::make_aura_bar_appearance();
	appearance.splits = 20;

	const auto max_value = 10000;
	appearance.label = ::make_aura_bar_label(in.config.drawing, range_test_bar_ratio, static_cast<int>(range_test_bar_ratio * max_value), max_value);

	::draw_hud_bar(
		in.get_drawer(),
		in.necessary_images,
		appearance,
		::make_aura_bar_rect(in.screen_size),
		range_test_bar_ratio,
		total_secs,
		in.config.damage_indication.white_damage_highlight_secs,
		bottom_bar_highlight,
		std::addressof(bottom_bar_particles),
		std::addressof(in.gui_fonts.gui),
		std::addressof(bottom_bar_label_particles)
	);
#else
	(void)in;
	(void)range_test_bar_ratio;
#endif
}

void test_scene_setup::draw_tutorial_hud(const draw_setup_gui_input& in) {
	using namespace augs::gui::text;

	if (!is_tutorial()) {
		return;
	}

	const auto& cosm = scene.world;
	const auto total_secs = cosm.get_total_seconds_passed(get_interpolation_ratio());

	const auto output = in.get_drawer();
	const auto highlight_base_secs = in.config.damage_indication.white_damage_highlight_secs;

	auto draw_bar = [&](
		hud_bar_particles_state& particles,
		hud_bar_highlight_state& highlight,
		const hud_bar_appearance& appearance,
		const ltrb bordered_rect,
		const float ratio,
		hud_bar_particles_state* const label_particles = nullptr
	) {
		::draw_hud_bar(
			output,
			in.necessary_images,
			appearance,
			bordered_rect,
			ratio,
			total_secs,
			highlight_base_secs,
			highlight,
			std::addressof(particles),
			std::addressof(in.gui_fonts.gui),
			label_particles
		);
	};

	const auto screen_size = in.screen_size;

	if (should_draw_bottom_progress_bar()) {
		uint32_t num_visited = 0;

		for (const auto& name : visited_tip_portals) {
			if (found_in(current_tip_portals, name)) {
				++num_visited;
			}
		}

		const auto ratio = static_cast<float>(num_visited) / current_tip_portals.size();

		/*
			Stretched over the whole screen's width,
			touching the bottom edge exactly - no padding.
		*/

		auto appearance = ::make_aura_bar_appearance();

		/* One segment per element. */
		appearance.splits = std::min(max_hud_bar_splits_v, static_cast<int>(current_tip_portals.size()));
		appearance.label = ::make_aura_bar_label(in.config.drawing, ratio, num_visited, static_cast<int>(current_tip_portals.size()));

		draw_bar(bottom_bar_particles, bottom_bar_highlight, appearance, ::make_aura_bar_rect(screen_size), ratio, std::addressof(bottom_bar_label_particles));
	}

	if (const auto stage = get_tutorial_stage_num_and_count()) {
		const auto& minimap = in.config.drawing.minimap;

		if (minimap.occupies_corner(minimap.position)) {
			const auto minimap_rect = ltrb(calc_minimap_rect(minimap, screen_size));

			const auto& font = in.gui_fonts.gui;
			const auto line_height = static_cast<int>(font.metrics.get_height());

			const auto bar_pad = 6;
			const auto label_pad = 8;
			const auto bar_h = tutorial_stage_bar_h_v;

			stage_block_height = bar_pad + bar_h + label_pad + line_height;

			const bool minimap_at_top =
				minimap.position == hud_corner_type::LEFT_TOP
				|| minimap.position == hud_corner_type::RIGHT_TOP
			;

			/*
				The bar sticks to the minimap's edge,
				with the label on the bar's far side.
			*/

			const auto bar_t =
				minimap_at_top ?
				minimap_rect.b + bar_pad :
				minimap_rect.t - bar_pad - bar_h
			;

			const auto label_t =
				minimap_at_top ?
				bar_t + bar_h + label_pad :
				bar_t - label_pad - line_height
			;

			auto appearance = hud_bar_appearance();

			/* The money bar's gold, mellowed the same way the bottom bar's green is. */
			appearance.color = rgba(183, 140, 22, 255);
			appearance.border_w = 2;
			appearance.particle_tint = 0.2f;

			/*
				One segment per level transition in the basic tutorial.
				The advanced one has too many stages, so group them instead:
				the smallest group of 2..4 stages dividing the count evenly,
				so that a whole segment fills exactly every k stages.
				With no even division (a prime count), fall back
				to one segment per transition to keep the boundaries aligned.
			*/

			const auto num_transitions = static_cast<int>(stage->second - 1);
			const bool is_basic = tutorial.level <= tutorial_last_basic_level_v;

			const auto advanced_splits = [&]() {
				for (int group = 2; group <= 4; ++group) {
					if (num_transitions % group == 0) {
						return num_transitions / group;
					}
				}

				return num_transitions;
			}();

			appearance.splits = std::min(max_hud_bar_splits_v, is_basic ? num_transitions : advanced_splits);

			/*
				Widened so that the bar's bright border
				lines up with the minimap's border.
			*/

			const auto bar_rect = ltrb(
				minimap_rect.l - minimap.border_thickness,
				bar_t,
				minimap_rect.r + minimap.border_thickness,
				bar_t + bar_h
			);

			/*
				The bar starts out empty on the first stage
				and gets full on the final one.
			*/

			const auto ratio =
				stage->second > 1 ?
				static_cast<float>(stage->first - 1) / (stage->second - 1) :
				1.0f
			;

			draw_bar(stage_bar_particles, stage_bar_highlight, appearance, bar_rect, ratio);

			/*
				Zero-based, so that the counter matches the bar exactly:
				Stage X/N = X out of N segments filled.
				The zeroth stage is just the welcome screen, not a real level.
			*/

			const auto label_text = formatted_string(
				stage->first == stage->second ?
				std::string("Final stage") :
				typesafe_sprintf("Stage %x/%x", stage->first - 1, stage->second - 1),
				style(font, in.config.arena_mode_gui.money_bar_color)
			);

			/*
				The label hugs the bar's screen-inward edge:
				its left edge when the minimap sits in a right corner,
				mirrored to its right edge when in a left corner.
			*/

			const bool minimap_at_left =
				minimap.position == hud_corner_type::LEFT_TOP
				|| minimap.position == hud_corner_type::LEFT_BOTTOM
			;

			const auto label_l =
				minimap_at_left ?
				bar_rect.r - get_text_bbox(label_text).x :
				bar_rect.l
			;

			print_stroked(
				output,
				vec2i(static_cast<int>(label_l), static_cast<int>(label_t)),
				label_text
			);
		}
	}
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