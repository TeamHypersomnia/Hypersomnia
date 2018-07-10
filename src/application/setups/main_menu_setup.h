#pragma once
#include <thread>
#include <mutex>
#include <future>
#include <string>
#include <optional>

#include <sol2/sol.hpp>

#include "augs/math/camera_cone.h"
#include "augs/templates/thread_templates.h"

#include "augs/misc/action_list/action_list.h"
#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/assets/all_logical_assets.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/organization/all_component_includes.h"

#include "view/viewables/all_viewables_defs.h"

#include "application/intercosm.h"

#include "application/setups/default_setup_settings.h"

#include "application/gui/menu/creators_screen.h"
#include "application/gui/main_menu_gui.h"
#include "application/setups/main_menu_settings.h"

struct config_lua_table;

namespace sol {
	class state;
}

class main_menu_setup : public default_setup_settings {
	std::shared_future<std::string> latest_news;
	vec2 latest_news_pos = { 0.f, 0.f };

	intercosm intro;

	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };
	cosmic_entropy total_collected_entropy;

	sol::table menu_config_patch;

	augs::sound_source menu_theme_source;
	std::optional<augs::single_sound_buffer> menu_theme;

#if TODO
	bool draw_menu_gui = false;
	bool roll_news = false;

	creators_screen creators;
#endif

	cosmic_movie_director director;
	unsigned initial_step_number = 0xdeadbeef;

	augs::action_list intro_actions;
	augs::action_list credits_actions;

public:
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
		return intro.local_test_subject;
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
		const necessary_images_in_atlas_map& necessarys,
		const augs::baked_font& gui_font,
		const vec2i screen_size
	) const;

	void control(const cosmic_entropy&) {
		return;
	}

	void accept_game_gui_events(const cosmic_entropy&) {
		return;
	}

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return {};
	}
};