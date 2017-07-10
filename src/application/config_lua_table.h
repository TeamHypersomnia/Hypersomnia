#pragma once
#include <string>
#include "augs/math/vec2.h"
#include "game/enums/input_recording_type.h"

#include "augs/pad_bytes.h"
#include "augs/graphics/rgba.h"
#include "augs/misc/basic_input_context.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/input_context_enums.h"
#include "game/view/game_drawing_settings.h"
#include "game/view/world_camera.h"

#include "application/content_generation/buttons_with_corners.h"
#include "application/config_structs/hotbar_settings.h"
#include "application/config_structs/debug_drawing_settings.h"

#include <imgui/imgui.h>

enum class launch_type {
	// GEN INTROSPECTOR enum class launch_type
	MAIN_MENU,

	LOCAL,
	LOCAL_DETERMINISM_TEST,

	DIRECTOR,
	CHOREOGRAPHIC,

	ONLY_CLIENT,
	ONLY_SERVER,

	CLIENT_AND_SERVER,
	TWO_CLIENTS_AND_SERVER,

	COUNT
	// END GEN INTROSPECTOR
};

struct config_gui_state {
	int active_pane = 0;
};

class config_lua_table {
public:
	config_lua_table() = default;
	config_lua_table(const std::string& config_lua_path);

	void save(
		const std::string& target_path
	) const;

	// GEN INTROSPECTOR class config_lua_table
	launch_type launch_mode = launch_type::LOCAL;
	input_recording_type input_recording_mode = input_recording_type::DISABLED;

	float recording_replay_speed = 1.f;

	unsigned determinism_test_cloned_cosmoi_count = 0;

	std::string window_name = "example";
	bool fullscreen = false;
	bool window_border = 1;
	vec2i window_position = vec2i(100, 10);
	unsigned bpp = 24;
	vec2i windowed_size = vec2i(1280, 768);

	bool check_content_integrity_every_launch = true;
	bool save_regenerated_atlases_as_binary = true;
	bool debug_regenerate_content_every_launch = false;
	unsigned packer_detail_max_atlas_size = 8192;

	bool debug_minimal_test_scene = false;

	bool debug_run_unit_tests = false;
	bool debug_log_successful_unit_tests = false;
	bool debug_break_on_unit_test_failure = false;

	bool enable_hrtf = true;
	unsigned max_number_of_sound_sources = 4096u;

	std::string audio_output_device;

	double gui_volume = 1.0;
	double sound_effects_volume = 1.0;
	double music_volume = 1.0;

	bool enable_cursor_clipping = false;

	std::string connect_address;
	unsigned short connect_port = 0;
	unsigned short server_port = 0;
	unsigned short alternative_port = 0;

	std::string nickname;
	std::string debug_second_nickname;

	vec2 mouse_sensitivity;

	unsigned default_tickrate = 60;

	unsigned jitter_buffer_ms = 0;
	unsigned client_commands_jitter_buffer_ms = 0;

	double interpolation_speed = 525.f;
	double misprediction_smoothing_multiplier = 1.2f;

	bool debug_randomize_entropies_in_client_setup = 0;
	unsigned debug_randomize_entropies_in_client_setup_once_every_steps = 0;

	bool server_launch_http_daemon = 0;
	unsigned short server_http_daemon_port = 0;
	std::string server_http_daemon_html_file_path;

	std::string db_path;
	std::string survey_num_file_path;
	std::string post_data_file_path;
	std::string last_session_update_link;

	std::string director_input_scene_entropy_path;
	std::string choreographic_input_scenario_path;
	std::string menu_intro_scene_entropy_path;
	std::string menu_intro_scene_cosmos_path;
	
	std::string menu_theme_path;

	double rewind_intro_scene_by_secs = 3.5;
	double start_menu_music_at_secs = 0.f;

	bool skip_credits = false;
	std::string latest_news_url;
	
	debug_drawing_settings debug;
	hotbar_settings hotbar;

	world_camera_settings camera_settings;
	game_drawing_settings drawing_settings;
	input_context controls;

	button_with_corners_input menu_button;
	button_with_corners_input hotbar_button;

	ImGuiStyle gui_style;
	// END GEN INTROSPECTOR

	vec2i get_screen_size() const;

	void perform_settings_gui(
		config_gui_state&,
		const config_lua_table& last_saved
	);
	
	launch_type get_launch_mode() const;
	input_recording_type get_input_recording_mode() const;
};

class viewing_session;

namespace augs {
	class renderer;
	class audio_device;

	namespace window {
		class glwindow;
	}
}

void apply_changes(
	const config_lua_table&,
	const config_lua_table&,
	augs::audio_device&,
	const bool force = false
);

void apply_changes(
	const config_lua_table&,
	const config_lua_table&,
	viewing_session&,
	const bool force = false
);

void apply_changes(
	const config_lua_table&,
	const config_lua_table&,
	augs::window::glwindow&,
	const bool force = false
);

void apply_changes(
	const config_lua_table&,
	const config_lua_table&,
	augs::renderer&,
	const bool force = false
);

template <class... Args>
void force_apply_changes(const config_lua_table& c, Args&&... args) {
	apply_changes(c, config_lua_table(), std::forward<Args>(args)..., true);
}

void sync_config_back(config_lua_table&, const augs::window::glwindow&);