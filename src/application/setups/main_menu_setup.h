#pragma once
#include <thread>
#include <mutex>
#include <future>
#include <string>

#include <sol2/sol.hpp>

#include "augs/templates/thread_templates.h"

#include "augs/misc/action_list/action_list.h"
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/assets/all_logical_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/organization/all_component_includes.h"

#include "view/viewables/viewables_loading_type.h"
#include "view/viewables/all_viewables_defs.h"

#include "application/workspace.h"

#include "application/gui/menu/creators_screen.h"
#include "application/gui/main_menu_gui.h"
#include "application/setups/main_menu_settings.h"

struct config_lua_table;

namespace sol {
	class state;
}

class main_menu_setup {
	std::shared_future<std::wstring> latest_news;
	vec2 latest_news_pos = { 0.f, 0.f };

	workspace intro;

	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	cosmic_entropy total_collected_entropy;

	sol::table menu_config_patch;

	augs::sound_source menu_theme_source;
	augs::single_sound_buffer menu_theme;

	bool draw_menu_gui = false;
	bool roll_news = false;

	cosmic_movie_director director;
	unsigned initial_step_number = 0xdeadbeef;

	creators_screen creators;

	augs::action_list intro_actions;
	augs::action_list credits_actions;
public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL_ONLY_ONCE;
	static constexpr bool handles_window_input = false;
	static constexpr bool handles_escape = false;

	main_menu_gui gui;

	main_menu_setup(sol::state&, const main_menu_settings);

	void query_latest_news(const std::string& url);

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return intro.world;
	}

	auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	auto get_viewed_character_id() const {
		return intro.locally_viewed;
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return intro.viewables;
	}

	void perform_custom_imgui() {
		return;
	}

	void launch_creators_screen();

	void customize_for_viewing(config_lua_table& config) const;
	void apply(const config_lua_table& config);

	template <class... Callbacks>
	void advance(
		const augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		latest_news_pos.x += frame_delta.per_second(50.f);

		timer.advance(frame_delta);

		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			total_collected_entropy.clear_dead_entities(intro.world);

			intro.advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void draw_overlays(
		const augs::drawer_with_default output,
		const necessary_images_in_atlas& necessarys,
		const augs::baked_font& gui_font,
		const vec2i screen_size
	) const;

	void control(const cosmic_entropy&) {
		return;
	}

	void accept_game_gui_events(const cosmic_entropy&) {
		return;
	}
};