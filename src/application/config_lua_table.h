#pragma once
#include <string>
#include "3rdparty/imgui/imgui.h"

#include "augs/pad_bytes.h"
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "augs/image/font.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/enums/game_intent_type.h"
#include "view/game_drawing_settings.h"
#include "view/audiovisual_state/world_camera.h"

// all settings structures stored by the config
#include "augs/window_framework/window_settings.h"
#include "augs/graphics/renderer_settings.h"
#include "augs/audio/audio_settings.h"
#include "game/debug_drawing_settings.h"

#include "view/game_gui/inventory_gui_intent_type.h"
#include "view/game_gui/elements/hotbar_settings.h"
#include "view/viewables/regeneration/content_regeneration_settings.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"
#include "view/audiovisual_state/systems/sound_system_settings.h"
#include "view/mode_gui/arena/arena_mode_gui_settings.h"
#include "view/faction_view_settings.h"
#include "view/gui_fonts.h"
#include "view/game_gui/elements/game_gui_settings.h"
#include "application/input/input_settings.h"

#include "test_scenes/test_scene_settings.h"

#include "application/debug_settings.h"
#include "application/session_settings.h"
#include "application/setups/main_menu_settings.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/server/server_start_input.h"
#include "application/setups/server/server_vars.h"
#include "application/setups/client/client_start_input.h"
#include "application/setups/client/client_vars.h"
#include "application/setups/client/lag_compensation_settings.h"
#include "application/app_intent_type.h"
#include "application/network/simulation_receiver_settings.h"
#include "application/performance_settings.h"

enum class launch_type {
	// GEN INTROSPECTOR enum class launch_type
	MAIN_MENU,

	TEST_SCENE,
	EDITOR,

	LOCAL_DETERMINISM_TEST,

	DIRECTOR,

	CLIENT,
	SERVER,

	CLIENT_AND_SERVER,
	TWO_CLIENTS_AND_SERVER,

	COUNT
	// END GEN INTROSPECTOR
};

struct config_read_error : public std::runtime_error {
	explicit config_read_error(
		const augs::path_type& path,
		const std::string& what
	) 
		: std::runtime_error(
			std::string("There was a problem reading "  + path.string() + ".\n" + what)
		)
	{}
};

namespace sol {
	class state;
}

struct config_lua_table {
	config_lua_table() = default;
	config_lua_table(sol::state&, const augs::path_type& config_lua_path);

	// GEN INTROSPECTOR struct config_lua_table
	launch_type launch_mode = launch_type::TEST_SCENE;
	bool log_to_live_file = false;
	bool perform_float_consistency_test = true;

	std::vector<std::string> official_servers;

	unit_tests_settings unit_tests;
	augs::window_settings window;
	augs::renderer_settings renderer;
	augs::audio_settings audio;
	augs::audio_volume_settings audio_volume;
	debug_drawing_settings debug_drawing;
	hotbar_settings hotbar;
	game_gui_settings game_gui;

	world_camera_settings camera;
	game_drawing_settings drawing;
	interpolation_settings interpolation;
	sound_system_settings sound;
	simulation_receiver_settings simulation_receiver;
	lag_compensation_settings lag_compensation;
	content_regeneration_settings content_regeneration;
	main_menu_settings main_menu;

	app_intent_map app_controls;
	game_intent_map game_controls;
	general_gui_intent_map general_gui_controls;
	inventory_gui_intent_map inventory_gui_controls;
	
	ImGuiStyle gui_style;
	debug_settings debug;
	session_settings session;
	test_scene_settings test_scene;
	editor_settings editor;
	all_gui_fonts_inputs gui_fonts;
	input_settings input;

	arena_mode_gui_settings arena_mode_gui;
	faction_view_settings faction_view;

	server_start_input default_server_start;
	server_vars server;
	server_solvable_vars server_solvable;
	private_server_vars private_server;
	augs::dedicated_server_input dedicated_server;

	client_start_input default_client_start;
	client_vars client;
	performance_settings performance;

#if TODO
	augs::path_type db_path;
	augs::path_type survey_num_file_path;
	augs::path_type post_data_file_path;
	std::string last_session_update_link;

	bool server_launch_http_daemon = 0;
	unsigned short server_http_daemon_port = 0;
	std::string server_http_daemon_html_file_path;
#endif

	// END GEN INTROSPECTOR

	bool operator==(const config_lua_table& b) const;
	bool operator!=(const config_lua_table& b) const;

	launch_type get_launch_mode() const;
	input_recording_type get_input_recording_mode() const;

	void save(sol::state&, const augs::path_type& target_path) const;
};