#pragma once
#include <cstdint>
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
#include "view/damage_indication_settings.h"
#include "view/hud_messages/hud_message_settings.h"
#include "application/input/input_settings.h"

#include "test_scenes/test_scene_settings.h"

#include "application/streamer_mode_flags.h"
#include "application/debug_settings.h"
#include "application/session_settings.h"
#include "application/setups/main_menu_settings.h"
#include "application/setups/debugger/debugger_settings.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/server/server_listen_input.h"
#include "application/setups/server/server_vars.h"
#include "application/setups/client/client_connect_string.h"
#include "application/setups/client/client_vars.h"
#include "application/setups/client/lag_compensation_settings.h"
#include "application/app_intent_type.h"
#include "application/network/simulation_receiver_settings.h"
#include "application/network/host_with_default_port.h"
#include "application/performance_settings.h"
#include "application/http_client/self_update_settings.h"
#include "application/masterserver/masterserver_settings.h"
#include "application/nat/nat_detection_settings.h"
#include "application/nat/nat_traversal_settings.h"
#include "application/activity_type.h"
#include "fp_consistency_tests.h"

enum class launch_type {
	// GEN INTROSPECTOR enum class launch_type
	MAIN_MENU,
	LAST_ACTIVITY,
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

bool operator==(const ImGuiStyle& a, const ImGuiStyle& b);

struct config_json_table {
	config_json_table() = default;
	config_json_table(const augs::path_type& config_lua_path);

	// GEN INTROSPECTOR struct config_json_table
	bool streamer_mode = false;
	streamer_mode_flags_data streamer_mode_flags;

	activity_type last_activity = activity_type::TUTORIAL;
	launch_type launch_at_startup = launch_type::LAST_ACTIVITY;

	bool skip_tutorial = false;
	bool log_to_live_file = false;
	bool remove_live_log_file_on_start = true;

	bool prompted_for_sign_in_once = false;

	std::string log_timestamp_format = std::string("[%m-%d-%y %H:%M:%S] ");

	float_consistency_test_settings float_consistency_test;

	std::string server_list_provider;
	std::string webrtc_signalling_server_url;

	nat_detection_settings nat_detection;
	nat_traversal_settings nat_traversal;

	masterserver_settings masterserver;

	self_update_settings self_update;
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
	
#if !HEADLESS
	ImGuiStyle gui_style;
#endif

	debug_settings debug;
	session_settings session;
	test_scene_settings test_scene;
	editor_settings editor;
#if BUILD_DEBUGGER_SETUP
	debugger_settings debugger;
#endif
	float ui_scale = 1.0f;
	all_gui_fonts_inputs gui_fonts;
	input_settings input;

	arena_mode_gui_settings arena_mode_gui;
	faction_view_settings faction_view;
	damage_indication_settings damage_indication;

	server_listen_input server_start;

	uint16_t num_ranked_servers = 0;
	uint16_t num_casual_servers = 0;

	server_vars server;
	server_private_vars server_private;
	augs::dedicated_server_input dedicated_server;

	client_connect_string client_connect;
	client_vars client;
	performance_settings performance;

	augs::maybe<hud_message_settings> hud_messages;

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

	bool operator==(const config_json_table& b) const = default;

	activity_type get_last_activity() const;
	input_recording_type get_input_recording_mode() const;

	void save_patch(
		const config_json_table& source,
		const augs::path_type& target_path,
		bool ephemeral_preamble
	) const;

	void load_patch(const augs::path_type& config_lua_path);
};