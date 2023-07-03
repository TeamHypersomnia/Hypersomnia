#pragma once
#include <thread>
#include <mutex>
#include <future>
#include <string>
#include <optional>

#include <sol/sol.hpp>

#include "augs/math/camera_cone.h"
#include "augs/templates/thread_templates.h"

#include "augs/misc/action_list/action_list.h"
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/assets/all_logical_assets.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/organization/all_component_includes.h"
#include "game/detail/render_layer_filter.h"

#include "view/viewables/all_viewables_defs.h"

#include "application/intercosm.h"

#include "application/setups/default_setup_settings.h"

#include "application/gui/menu/creators_screen.h"
#include "application/gui/main_menu_gui.h"
#include "application/setups/main_menu_settings.h"
#include "game/modes/test_mode.h"
#include "application/setups/setup_common.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "augs/network/netcode_sockets.h"

struct self_update_result;
struct config_lua_table;
struct draw_setup_gui_input;

namespace sol {
	class state;
}

struct packaged_official_content;

class main_menu_setup : public default_setup_settings {
	std::shared_future<std::string> latest_news;
	vec2 latest_news_pos = { 1920.f, 0.f };

	intercosm scene;
	test_mode mode;
	test_mode_ruleset ruleset;
	entity_id viewed_character_id;

	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	sol::table menu_config_patch;

	augs::sound_source menu_theme_source;
	std::optional<augs::single_sound_buffer> menu_theme;

	augs::path_type current_arena_folder;

#if TODO
	bool draw_menu_gui = false;
	bool roll_news = false;

	creators_screen creators;
#endif

	unsigned initial_step_number = 0xdeadbeef;

	augs::action_list intro_actions;
	augs::action_list credits_actions;

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;

	main_menu_setup(sol::state&, const packaged_official_content&, const main_menu_settings);

	void query_latest_news(const std::string& url);

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return scene.world;
	}

	auto get_interpolation_ratio() const {
		return timer.next_step_progress_fraction(get_viewed_cosmos().get_fixed_delta().in_seconds<double>());
	}

	entity_id get_viewed_character_id() const;

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
		return custom_imgui_result::NONE;
	}

	auto escape() {
		return setup_escape_result::IGNORE;
	}

	void launch_creators_screen();

	void customize_for_viewing(config_lua_table& config) const;
	void apply(const config_lua_table& config);

	auto get_inv_tickrate() const {
		return get_viewed_cosmos().get_fixed_delta().in_seconds<double>();
	}

	template <class C>
	void advance(
		const setup_advance_input in,
		const C& callbacks
	) {
		latest_news_pos.x -= in.frame_delta.per_second(50.f);

		timer.advance(in.frame_delta);

		auto steps = timer.extract_num_of_logic_steps(get_inv_tickrate());

		while (steps--) {
			mode_entropy entropy;

			mode.advance(
				{ ruleset, scene.world },
				entropy,
				callbacks,
				solve_settings()
			);
		}
	}

	void draw_overlays(
		const self_update_result& last_update_result,
		const augs::drawer_with_default output,
		const necessary_images_in_atlas_map& necessarys,
		const augs::baked_font& gui_font,
		const vec2i screen_size
	) const;

	template <class T>
	void control(const T&) {
		return;
	}

	void accept_game_gui_events(const game_gui_entropy_type&) {
		return;
	}

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return current_arena_folder;
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&) {}

	void ensure_handler() {}
	bool requires_cursor() const { return false; }

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