#pragma once
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

#include "application/setups/editor/gui/editor_inspector_gui.h"
#include "application/setups/editor/gui/editor_hierarchy_gui.h"
#include "application/setups/editor/gui/editor_project_files_gui.h"

struct config_lua_table;
struct draw_setup_gui_input;

namespace sol {
	class state;
}

struct editor_gui {
	// GEN INTROSPECTOR struct editor_gui
	editor_inspector_gui inspector = std::string("Inspector");
	editor_hierarchy_gui hierarchy = std::string("Hierarchy");
	editor_project_files_gui project_files = std::string("Project files");
	// END GEN INTROSPECTOR
};

class editor_setup : public default_setup_settings {
	test_mode mode;
	test_mode_ruleset ruleset;

	intercosm scene;
	entropy_accumulator total_collected;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	entity_id viewed_character_id;
	mode_player_id local_player_id;

	editor_gui gui;

	void load_gui_state();
	void save_gui_state();

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;

	editor_setup();
	editor_setup(const augs::path_type& project_path);
	
	~editor_setup();

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta().in_seconds<double>());
	}

	auto get_viewed_character_id() const {
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

	void perform_main_menu_bar(perform_custom_imgui_input);
	custom_imgui_result perform_custom_imgui(perform_custom_imgui_input);

	void customize_for_viewing(config_lua_table&) const;

	void apply(const config_lua_table&) {
		return;
	}

	auto escape() {
		return setup_escape_result::IGNORE;
	}

	auto get_inv_tickrate() const {
		return get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
	}

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		timer.advance(in.frame_delta);

		auto steps = timer.extract_num_of_logic_steps(get_inv_tickrate());

		while (steps--) {
			const auto total = total_collected.extract(
				get_viewed_character(), 
				local_player_id, 
				in.make_accumulator_input()
			);

			mode.advance(
				{ ruleset, scene.world },
				total,
				callbacks,
				solve_settings()
			);
		}
	}

	template <class T>
	void control(const T& t) {
		total_collected.control(t);
	}

	void accept_game_gui_events(const game_gui_entropy_type&) {}

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return {};
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&) {}

	void ensure_handler() {}
	bool requires_cursor() const { return false; }

	const entropy_accumulator& get_entropy_accumulator() const {
		return total_collected;
	}

	template <class F>
	void on_mode_with_input(F&& callback) const {
		callback(mode, test_mode::const_input { ruleset, scene.world });
	}

	auto get_game_gui_subject_id() const {
		return get_viewed_character_id();
	}

	std::nullopt_t get_new_player_metas() {
		return std::nullopt;
	}

	std::nullopt_t get_new_ad_hoc_images() {
		return std::nullopt;
	}

	const arena_player_metas* find_player_metas() const {
		return nullptr;
	}

	void after_all_drawcalls(game_frame_buffer&) {}
	void do_game_main_thread_synced_op(renderer_backend_result&) {}
};