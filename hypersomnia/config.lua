config_table = {
	launch_mode = "LOCAL",
	
	input_recording_mode = "LIVE",

	-- initial replaying speed of the machine entropies recording
	recording_replay_speed = 6,

	-- see LOCAL_DETERMINISM_TEST
	determinism_test_cloned_cosmoi_count = 2,

	-- game window settings
	window_name = "example",
	fullscreen = false,
	window_border = 1,
	window_x = 100,
	window_y = 10,
	bpp = 24,
	resolution_w = 1280,
	resolution_h = 768,
	doublebuffer = true,

	check_content_integrity_every_launch = true,

	-- 1 - more hdd space required, but launching the game is way faster
	save_regenerated_atlases_as_binary = true,

	-- 1 always regenerates the entire content. only for debugging pruposes - the game will take a lot longer to launch every time.
	debug_regenerate_content_every_launch = false,

	-- detail value. so that the regenerator does not have to query actual value from opengl.
	packer_detail_max_atlas_size = 8192,

	-- if true, runs unit tests on every launch.
	debug_run_unit_tests = true,

	-- if true, logs all unit tests that succeed, not just failures.
	debug_log_successful_unit_tests = false,

	-- if true, breaks on the first failure.
	debug_break_on_unit_test_failure = true,

	-- should enable head-related transfer function for OpenAL?
	enable_hrtf = true,

	-- maximum number of audible sound effects
	max_number_of_sound_sources = 4096,

	-- value of "" means the default audio device will be used
	audio_output_device = "",
	-- OpenAL Soft on Line 1 (Virtual Audio Cable)
	
	-- volume settings
	sound_effects_volume = 0.1,
	music_volume = 1,

	-- Flag. 1 disables the cursor clipping so that it is easier to mark a breakpoint, for example. 0 is for normal playing.
	debug_disable_cursor_clipping = false,
	
	-- vec2. Sensitivity of mouse movement in-game.
	mouse_sensitivity = { x = 1.5, y = 1.5 },
	
	-- Network variables. See launch_mode for details.
	connect_address = "192.168.1.8",
	connect_port = 13372,

	server_port = 13372,

	alternative_port = 13373,
	
	-- Client-chosen nickname of the controlled character.
	nickname = "Sentinel",

	-- Client-chosen nickname of the second controlled character used in launch_mode.TWO_CLIENTS_AND_SERVER.
	debug_second_nickname = "Billan",
	
	-- Frequency of the simulation. 1/tickrate equals the fixed delta time in seconds, so tickrate = 60 means that the logical step advances the simulation about around 16 milliseconds.
	default_tickrate = 60,

	-- Client-side jitter buffer time to preserve smooth display of the past. The bigger the value, the bigger the lag.
	jitter_buffer_ms = 50,

	-- Server-side jitter buffer time for client commands. Useful for lag simulation.
	client_commands_jitter_buffer_ms = 0,
	
	interpolation_speed = 525,
	misprediction_smoothing_multiplier = 1.2,

	-- Used by the server to inject random inputs to the other players to examine and test lag compensation strategies.
	debug_randomize_entropies_in_client_setup = true,

	-- How often the above input injection happens. The less it is, the more erratic the movements are.
	debug_randomize_entropies_in_client_setup_once_every_steps = 1,

	-- Flag. 1 will launch a http daemon on the localhost in a separate thread which samples the server statistics. Used as a widget on http://hypersomnia.pl
	server_launch_http_daemon = true,

	-- What port to open the web daemon on. Recommended value: 80.
	server_http_daemon_port = 80,

	-- Format of the broadcasted widget.
	server_http_daemon_html_file_path = "web/session_report.html",

	-- Path to server's private information
	db_path = "P:/Projects/db/",

	-- Survey number
	survey_num_file_path = "survey_num.in",
	
	-- Post data for some requests
	post_data_file_path = "post.txt",
	
	-- Will refresh last session time on the blog
	last_session_update_link = "patrykcysarz.pl/comment-system/web/stats/last-update/set",

	-- Intro scene path to be opened with DIRECTOR mode
	director_input_scene_entropy_path = "director/sequence_3.ent",

	-- Input scenario for choreographic launch mode
	choreographic_input_scenario_path = "choreographic/gameplay_1.chg", 
	
	-- Main menu intro scene path recorded with DIRECTOR mode
	menu_intro_scene_entropy_path = "director/menu_intro.ent",

	-- Menu theme path
	menu_theme_path = "hypersomnia/music/menu_theme.flac",

	-- Menu intro timing settings
	rewind_intro_scene_by_secs = 3.5,
	start_menu_music_at_secs = 63.5 - 22.5,

	skip_credits = false,
	
	-- Url for the rolling news bar in the main menu
	latest_news_url = "http://hypersomnia.pl/latest_post/",

	-- hotbar appearance settings
	hotbar = {
		increase_inside_alpha_when_selected = false,
		colorize_inside_when_selected = true,
		primary_selected_color = { r = 0, g = 255, b = 255, a = 255 },
		secondary_selected_color = { r = 86, g = 156, b = 214, a = 255 }
	},

	-- debug drawing settings

	debug = {
		drawing_enabled = true,
		draw_colinearization = false,
		draw_forces = true,
		draw_friction_field_collisions_of_entering = false,
		draw_explosion_forces = false,
		draw_visibility = false
	}
}
