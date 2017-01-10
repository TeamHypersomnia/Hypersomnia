launch_type = {
	MAIN_MENU = 0,

	LOCAL = 1,
	LOCAL_DETERMINISM_TEST = 2,
	-- Remember that in-game replayed results may look different due to disabled interpolation
	-- in the director mode and different behaviour of audiovisual response systems.
	DIRECTOR = 3,

	ONLY_CLIENT = 4,
	ONLY_SERVER = 5,

	CLIENT_AND_SERVER = 6,

	TWO_CLIENTS_AND_SERVER = 7
}

recording_type = {
	DISABLE = 0,
	LIVE_WITH_BUFFER = 1,
	LIVE = 2
}

config_table = {
	launch_mode = launch_type.MAIN_MENU,
	
	input_recording_mode = recording_type.LIVE,
	recording_replay_speed = 1,

	determinism_test_cloned_cosmoi_count = 2,

	window_name = "example",
	fullscreen = 0,
	window_border = 1,
	window_x = 100,
	window_y = 10,
	bpp = 24,
	resolution_w = 1280,
	resolution_h = 768,
	doublebuffer = 1,

	enable_hrtf = 1,

	sound_effects_volume = 1,
	music_volume = 1,

	debug_disable_cursor_clipping = 0,
	
	mouse_sensitivity = vec2(1.5, 1.5),
	
	connect_address = "192.168.1.8",
	connect_port = 13372,

	server_port = 13372,

	alternative_port = 13373,
	
	nickname = "Pythagoras",
	debug_second_nickname = "Billan",
	
	tickrate = 60,

	jitter_buffer_ms = 50,
	client_commands_jitter_buffer_ms = 0,
	
	interpolation_speed = 525,
	misprediction_smoothing_multiplier = 1.2,

	debug_var = 0,
	debug_randomize_entropies_in_client_setup = 1,
	debug_randomize_entropies_in_client_setup_once_every_steps = 1,

	server_launch_http_daemon = 1,
	server_http_daemon_port = 80,
	server_http_daemon_html_file_path = "web/session_report.html",

	db_path = "P:/Projects/db/",
	survey_num_file_path = "survey_num.in",
	post_data_file_path = "post.txt",
	last_session_update_link = "patrykcysarz.pl/comment-system/web/stats/last-update/set",

	director_scenario_filename = "director/menu_intro.ent",
	menu_intro_scenario_filename = "director/menu_intro.ent",

	menu_theme_filename = "hypersomnia/music/menu_theme.flac",

	rewind_intro_scene_by_secs = 3.5,
	start_menu_music_at_secs = 63.5 - 22.5,

	skip_credits = 0,
	latest_news_url = "http://hypersomnia.pl/latest_post/"
}
