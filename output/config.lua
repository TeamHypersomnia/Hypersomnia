launch_modes = {
	LOCAL = 0,
	LOCAL_DETERMINISM_TEST = 1,

	ONLY_CLIENT = 2,
	ONLY_SERVER = 3,

	CLIENT_AND_SERVER = 4,

	TWO_CLIENTS_AND_SERVER = 5
}

config_table = {
	launch_mode = launch_modes.LOCAL,
	
	determinism_test_cloned_cosmoi_count = 2,

	window_name = "example",
	fullscreen = 0,
	window_border = 1,
	window_x = 400,
	window_y = 0,
	bpp = 24,
	resolution_w = 1000,
	resolution_h = 1000,
	doublebuffer = 1,

	debug_disable_cursor_clipping = 0,
	
	mouse_sensitivity = vec2(1.5, 1.5),
	
	connect_address = "192.168.1.2",
	connect_port = 13372,

	server_port = 13372,

	alternative_port = 13373,
	
	nickname = "Pythagoras",
	debug_second_nickname = "BOT Ferrero Roche",
	
	tickrate = 60,

	jitter_buffer_ms = 50,
	client_commands_jitter_buffer_ms = 130,
	
	interpolation_speed = 525,

	debug_var = 0,
	debug_randomize_entropies_in_client_setup = 1,
	debug_randomize_entropies_in_client_setup_once_every_steps = 1,

	server_launch_http_daemon = 1,
	server_http_daemon_port = 80,
	server_http_daemon_html_file_path = "web/session_report.html"
}

if config_table.debug_disable_cursor_clipping == 0 then
	set_cursor_visible(0)
end

if config_table.fullscreen == 1 then
	config_table.resolution_w = get_display().w
	config_table.resolution_h = get_display().h
	set_display(config_table.resolution_w, config_table.resolution_h, 32)
end

enabled_window_border = 1

if config_table.window_border == 0 or config_table.fullscreen == 1 then
	enabled_window_border = 0
end

global_gl_window:create(
	rect_xywh_i(config_table.window_x, 
				config_table.window_y, 
				config_table.resolution_w, 
				config_table.resolution_h), 
	enabled_window_border, 
	config_table.window_name, 
	config_table.doublebuffer, 
	config_table.bpp)
	
global_gl_window:set_vsync(0)

global_gl_window:set_as_current()
