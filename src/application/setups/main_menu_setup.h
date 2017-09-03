#pragma once
#include <thread>
#include <mutex>
#include <string>

#include <sol2/sol.hpp>

#include "augs/misc/action_list.h"
#include "augs/misc/fixed_delta_timer.h"

#include "game/assets/all_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include "application/gui/menu/creators_screen.h"
#include "application/gui/main_menu_gui.h"
#include "application/setups/main_menu_settings.h"

struct config_lua_table;

class main_menu_setup {
	cosmos intro_scene;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);
	cosmic_entropy total_collected_entropy;
	all_logical_assets logical_assets;
	all_viewable_defs viewable_defs;

	entity_id viewed_character_id;
	sol::table menu_config_patch;

	augs::sound_source menu_theme_source;
	augs::single_sound_buffer menu_theme;

	std::thread latest_news_query;

	std::mutex news_mut;
	std::wstring downloaded_news_text;

	bool draw_menu_gui = false;
	bool roll_news = false;

	cosmic_movie_director director;
	unsigned initial_step_number = 0xdeadbeef;

	creators_screen creators;

	augs::action_list intro_actions;
	augs::action_list credits_actions;
public:
	static constexpr auto loading_strategy = viewables_loading_type::ALWAYS_HAVE_ALL_LOADED;
	static constexpr bool can_viewables_change = false;

	main_menu_gui gui;

	main_menu_setup(const main_menu_settings);

	auto get_audiovisual_speed() const {
		return timer.get_stepping_speed_multiplier();
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(intro_scene.get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return viewed_character_id;
	}

	const auto& get_viewed_cosmos() const {
		return intro_scene;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return viewable_defs;
	}

	void perform_custom_imgui() {
		return;
	}

	void launch_creators_screen();

	void customize_for_viewing(config_lua_table& config);
	void apply(const config_lua_table& config);

	template <class F, class G>
	void advance(
		F&& advance_audiovisuals,
		G&& step_post_solve
	) {
		auto steps = timer.count_logic_steps_to_perform(intro_scene.get_fixed_delta());

		if (!steps) {
			advance_audiovisuals();
		}

		while (steps--) {
			augs::renderer::get_current().clear_logic_lines();

			intro_scene.advance(
				{ total_collected_entropy, logical_assets },
				[](auto) {},
				std::forward<G>(step_post_solve)
			);

			total_collected_entropy.clear();
			advance_audiovisuals();
		}
	}

	void draw_overlays(
		const augs::drawer_with_default output,
		const necessary_images_in_atlas& necessarys,
		const augs::baked_font& gui_font,
		const vec2i screen_size
	) const;

	void control(
		augs::local_entropy& entropy,
		const input_context&
	) {
		return;
	}

	void accept_game_gui_events(const cosmic_entropy&) {
		return;
	}
};