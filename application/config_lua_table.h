#pragma once
#include <string>
#include "augs/math/vec2.h"
#include "game/enums/input_recording_type.h"

#include "augs/padding_byte.h"
#include "augs/graphics/pixel.h"

#include "game/transcendental/entity_handle_declaration.h"

class game_window;

namespace augs {
	class lua_state_raii;
}

class config_lua_table {
public:
	int launch_mode = 0;
	int input_recording_mode = 0;

	float recording_replay_speed = 1.f;

	unsigned determinism_test_cloned_cosmoi_count = 0;
	
	bool enable_hrtf = true;
	unsigned max_number_of_sound_sources = 0u;

	std::string audio_output_device;

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

	void get_values(augs::lua_state_raii&);

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
};