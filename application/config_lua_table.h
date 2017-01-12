#pragma once
#include <string>
#include "augs/math/vec2.h"
#include "game/enums/input_recording_type.h"
#include "augs/scripting/lua_state_wrapper.h"

#include <thread>
#include <mutex>

#include "augs/padding_byte.h"
#include "augs/graphics/pixel.h"

#include "game/transcendental/entity_handle_declaration.h"

class game_window;

class config_lua_table {
	augs::lua_state_wrapper lua;
	std::mutex lua_mutex;

	template<class T>
	void get_config_value(const std::string& field, T& into) {
		std::unique_lock<std::mutex> lock(lua_mutex);

		into = luabind::object_cast<T>(luabind::globals(lua.raw)["config_table"][field]);
	}

	void get_config_value(const std::string& field, bool& into);

public:
	int launch_mode = 0;
	int input_recording_mode = 0;

	float recording_replay_speed = 1.f;

	unsigned determinism_test_cloned_cosmoi_count = 0;
	
	bool enable_hrtf = true;

	float sound_effects_volume = 1.f;
	float music_volume = 1.f;

	bool debug_disable_cursor_clipping = false;

	std::string connect_address;
	unsigned short connect_port = 0;
	unsigned short server_port = 0;
	unsigned short alternative_port = 0;

	std::string nickname;
	std::string debug_second_nickname;

	vec2 mouse_sensitivity;

	unsigned tickrate = 0;

	unsigned jitter_buffer_ms = 0;
	unsigned client_commands_jitter_buffer_ms = 0;

	float interpolation_speed = 0;
	float misprediction_smoothing_multiplier = 0.5;

	int debug_var = 0;
	bool debug_randomize_entropies_in_client_setup = 0;
	unsigned debug_randomize_entropies_in_client_setup_once_every_steps = 0;

	bool server_launch_http_daemon = 0;
	unsigned short server_http_daemon_port = 0;
	std::string server_http_daemon_html_file_path;

	std::string db_path;
	std::string survey_num_file_path;
	std::string post_data_file_path;
	std::string last_session_update_link;

	std::string director_scenario_filename;
	std::string menu_intro_scenario_filename;

	std::string menu_theme_filename;

	float rewind_intro_scene_by_secs = 3.5;
	float start_menu_music_at_secs = 0.f;

	bool skip_credits = false;
	std::string latest_news_url;

	struct hotbar_settings {
		bool increase_inside_alpha_when_selected = false;
		bool colorize_inside_when_selected = true;
		padding_byte pad[2];
		
		rgba primary_selected_color = rgba(0, 255, 255, 255);
		rgba secondary_selected_color = rgba(86, 156, 214, 255); 
	} hotbar;

	config_lua_table();

	void get_values();

	void call_config_script(const std::string& filename, const std::string& alternative_filename);
	void call_window_script(game_window&, const std::string& filename);

	enum class launch_type {
		INVALID,

		MAIN_MENU,

		LOCAL,
		LOCAL_DETERMINISM_TEST,
		DIRECTOR,

		ONLY_CLIENT,
		ONLY_SERVER,

		CLIENT_AND_SERVER,
		TWO_CLIENTS_AND_SERVER,
	};
	
	launch_type get_launch_mode() const;
	input_recording_type get_input_recording_mode() const;

	void update_configuration_for_entity(const entity_handle) const;
};