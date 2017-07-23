#pragma once
#include "augs/global_libraries.h"
#include "augs/window_framework/window.h"
#include "augs/gui/formatted_text.h"

#include "application/menu/menu_root.h"
#include "application/menu/menu_context.h"

#include "application/setups/main_menu_setup.h"
#include "application/setups/local_setup.h"
//#include "application/setups/determinism_test_setup.h"
//#include "application/setups/two_clients_and_server_setup.h"
//#include "application/setups/client_setup.h"
//#include "application/setups/server_setup.h"
//#include "application/setups/director_setup.h"
//#include "application/setups/choreographic_setup.h"

#include "game/transcendental/types_specification/all_component_includes.h"

#include "game/assets/assets_manager.h"
#include "game/view/audiovisual_state.h"

using game_window = augs::window;

namespace augs {
	struct machine_entropy;

	namespace network {
		class client;
	}

	class renderer;
}

struct settings_gui_state {
	int active_pane = 0;
};

class viewing_session {
	settings_gui_state settings_gui;
public:
	config_lua_table config;
	config_lua_table last_saved_config;
	
	const std::string config_path_for_saving;

	augs::global_libraries libraries;
	game_window window;
	augs::renderer gl;
	augs::audio_context audio_context;
#if !ONLY_ONE_GLOBAL_ASSETS_MANAGER
	assets_manager manager;
#endif

	audiovisual_state audiovisuals;

	using setup_variant = std::variant<local_setup>;
	
	std::optional<main_menu_setup> main_menu;
	std::optional<setup_variant> current_setup;

	bool show_settings = false;
	bool show_ingame_menu = false;

	augs::timer gui_timer;
	augs::timer imgui_timer;

	augs::event::state state;
	
	mutable session_profiler profiler;

	viewing_session(
		const config_lua_table&,
		const std::string& config_path_for_saving
	);

	void set_screen_size(const vec2i);

	bool switch_between_gui_and_back(const augs::machine_entropy::local_type&);
	
	void perform_imgui_pass(
		augs::machine_entropy::local_type&,
		const augs::delta dt
	);
	
	void perform_ingame_menu();
	void perform_settings_gui();

	void sync_back(config_lua_table& into);
	void apply(const config_lua_table& new_config);

	void fetch_gui_events(
		const const_entity_handle root,
		augs::machine_entropy::local_type&
	);

	void fetch_developer_console_intents(game_intent_vector&);
	void fetch_session_intents(game_intent_vector&);

	decltype(auto) get_standard_post_solve() {
		return [this](const const_logic_step step) {
			audiovisuals.standard_post_solve(step);
		};
	}
	
	void view(
		augs::renderer& renderer,
		const cosmos& cosmos,
		const entity_id viewed_character,
		const visible_entities&,
		const double interpolation_ratio,
		const augs::gui::text::formatted_string& custom_log = augs::gui::text::formatted_string()
	) const;

	void draw_text_at_left_top(
		augs::renderer& renderer,
		const augs::gui::text::formatted_string&
	) const;

	void get_visible_entities(
		visible_entities& into,
		const cosmos&
	);
	
	std::wstring get_profile_summary() const;

	void draw_color_overlay(
		augs::vertex_triangle_buffer&, 
		const rgba
	) const;
};