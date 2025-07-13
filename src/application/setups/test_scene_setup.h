#pragma once
#include <cstdint>
#include <optional>
#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/math/camera_cone.h"

#include "game/detail/render_layer_filter.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/test_mode.h"

#include "test_scenes/test_scene_settings.h"

#include "application/intercosm.h"
#include "application/setups/default_setup_settings.h"

#include "application/debug_settings.h"
#include "application/input/entropy_accumulator.h"
#include "application/setups/setup_common.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "view/mode_gui/arena/arena_gui_mixin.h"
#include "application/network/network_common.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/arena/scene_entity_to_node_map.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "steam_rich_presence_pairs.h"

#include "application/arena/synced_dynamic_vars.h"

struct config_json_table;
struct draw_setup_gui_input;

namespace sol {
	class state;
}

struct packaged_official_content;

enum class test_scene_type {
	TUTORIAL,
	SHOOTING_RANGE
};

struct tutorial_state {
	uint32_t level = 0;
	bool challenge = false;
};

template <bool C>
using test_arena_handle = online_arena_handle<C>;

class test_scene_setup : public default_setup_settings, public arena_gui_mixin<test_scene_setup> {
	using arena_gui_base = arena_gui_mixin<test_scene_setup>;

	const packaged_official_content& official;

	all_rulesets_variant ruleset;
	all_modes_variant current_mode_state;

	mutable std::string nickname;

	cosmos_solvable_significant dummy_clean_round_state;
	synced_dynamic_vars dummy_dynamic_vars;
	uint32_t clean_step_number = 0;

	entity_id range_entry_portal;

	intercosm scene;
	entropy_accumulator total_collected;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	entity_id viewed_character_id;
	mode_player_id local_player_id;

	editor_project project;
	scene_entity_to_node_map entity_to_node;
	name_to_node_map_type name_to_node;

	augs::path_type current_arena_folder;
	test_scene_type type;

	tutorial_state tutorial;
	uint32_t max_tutorial_level = 0;

	bool should_init_level = false;
	float restart_arena_in_ms = -1;

	std::unordered_map<std::string, entity_id> opponents;
	bool restart_requested = false;

	std::optional<custom_imgui_result> special_result;

	std::vector<std::byte> avatar_bytes;
	arena_player_metas player_metas;
	bool rebuild_player_meta_viewables = true;

	template <class H, class S>
	static decltype(auto) get_arena_handle_impl(S& self) {
		return H {
			self.current_mode_state,
			self.scene,
			self.scene.world,
			self.ruleset,
			self.dummy_clean_round_state,
			self.dummy_dynamic_vars
		};
	}

	void init(test_scene_type type);

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;

	test_scene_setup(
		std::string nickname,
		std::vector<std::byte> avatar_bytes,
		const packaged_official_content&,
		const test_scene_type type
	);

	float speed = 1.0f;

	auto get_audiovisual_speed() const {
		return speed;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		return timer.next_step_progress_fraction(get_viewed_cosmos().get_fixed_delta().in_seconds<double>());
	}

	auto get_viewed_character_id() const {
		if (range_entry_portal.is_set()) {
			return range_entry_portal;
		}

		return viewed_character_id;
	}

	auto get_controlled_character_id() const {
		return get_viewed_character_id();
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return scene.viewables;
	}

	auto perform_custom_imgui(perform_custom_imgui_input) {
		if (special_result) {
			return *special_result;
		}

		return custom_imgui_result::NONE;
	}

	void customize_for_viewing(config_json_table&) const;

	void apply(const config_json_table&) {
		return;
	}

	setup_escape_result escape();

	auto get_inv_tickrate() const {
		return get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
	}

	bool post_solve(const const_logic_step step);
	void pre_solve(const logic_step step);

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		auto dt = in.frame_delta;
		dt *= speed;
		timer.advance(dt);

		auto steps = timer.extract_num_of_logic_steps(get_inv_tickrate());

		if (range_entry_portal.is_set()) {
			steps = std::min(steps, 1u);
		}

		auto get_solve_settings = [&]() {
			solve_settings settings;

			if (scene.world.get_total_steps_passed() <= clean_step_number + 1) {
				settings.play_transfer_sounds = false;
			}

			//settings.drop_weapons_if_empty = false;
			return settings;
		};

		while (steps--) {
			auto accum_in = in.make_accumulator_input();

			const auto total = total_collected.extract(
				get_viewed_character(), 
				local_player_id, 
				accum_in
			);


			get_arena_handle().on_mode_with_input(
				[&](auto& mode, const auto& input) {
					bool restarted = false;

					auto with_post_solve = solver_callbacks(
						[&](const logic_step step) {
							this->pre_solve(step);
							callbacks.pre_solve(step);
						},
						[&](const const_logic_step step) {
							if (this->post_solve(step)) {
								/* Prevent post-solving the old step if we've only just restarted. */
								restarted = true;
								return;
							}

							callbacks.post_solve(step);
						},
						callbacks.post_cleanup
					);

					mode.advance(
						input,
						total,
						with_post_solve,
						get_solve_settings()
					);

					if (restarted) {
						/* Advance once normally */

						mode.advance(
							input,
							total,
							callbacks,
							get_solve_settings()
						);
					}
				}
			);
		}
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
		arena_gui_base::arena_gui.buy_menu.show = false;
		arena_gui_base::arena_gui.choose_team.show = false;
	}

	void accept_game_gui_events(const game_gui_entropy_type&);

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return current_arena_folder;
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&);

	void ensure_handler() {}
	bool requires_cursor() const { return false; }

	const entropy_accumulator& get_entropy_accumulator() const {
		return total_collected;
	}

	auto get_game_gui_subject_id() const {
		return get_viewed_character_id();
	}


	std::nullopt_t get_new_ad_hoc_images() {
		return std::nullopt;
	}

	std::optional<arena_player_metas> get_new_player_metas();

	void set_new_avatar(std::vector<std::byte>);

	const arena_player_metas* find_player_metas() const {
		return std::addressof(player_metas);
	}

	void after_all_drawcalls(game_frame_buffer&) {}
	void do_game_main_thread_synced_op(renderer_backend_result&) {}

	bool handle_input_before_imgui(
		handle_input_before_imgui_input in
	);

	bool handle_input_before_game(
		const handle_input_before_game_input
	);

	bool is_gameplay_on() const { return true; }

	mode_player_id get_local_player_id() const {
		return local_player_id;
	}

	test_arena_handle<false> get_arena_handle();
	test_arena_handle<true> get_arena_handle() const;

	template <class F>
	decltype(auto) on_mode_with_input(F&& callback) const {
		return get_arena_handle().on_mode_with_input(std::forward<F>(callback));
	}

	bool is_tutorial() const {
		return type == test_scene_type::TUTORIAL;
	}

	void restart_mode();
	void restart_arena();

	void do_tutorial_logic(logic_step);

	template <class T>
	T* find(const std::string& name);

	template <class T>
	T* find(const editor_node_id& id);

	template <class T>
	T* find(const entity_id& id);

	template <class T>
	const T* find(const std::string& name) const;

	template <class T>
	const T* find(const editor_node_id& id) const;

	template <class T>
	const T* find(const entity_id& id) const;

	entity_id to_entity(const std::string&) const;
	entity_handle to_handle(const std::string&);
	const_entity_handle to_handle(const std::string&) const;

	bool is_killed(const std::string& name) const;
	void remove(logic_step step, const std::string& name);

	void request_checkpoint_restart() { restart_requested = true; }

	auto get_paths() const {
		return editor_project_paths(current_arena_folder);
	}

	void get_steam_rich_presence_pairs(steam_rich_presence_pairs&) const;

	void set_tutorial_level(uint32_t level);
	void set_tutorial_surfing_challenge();

	std::string get_browser_location() const;
	std::string get_scoreboard_caption() const;
};