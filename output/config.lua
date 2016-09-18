launch_modes = {
	LOCAL = 0,
	LOCAL_DETERMINISM_TEST = 1,

	ONLY_CLIENT = 2,
	ONLY_SERVER = 3,

	CLIENT_AND_SERVER = 4,

	TWO_CLIENTS_AND_SERVER = 5
}

config_table = {
	launch_mode = launch_modes.CLIENT_AND_SERVER,
	
	determinism_test_cloned_cosmoi_count = 2,

	window_name = "example",
	fullscreen = 0,
	window_border = 0,
	window_x = 0,
	window_y = 0,
	bpp = 24,
	resolution_w = 1000,
	resolution_h = 720,
	doublebuffer = 1,
	
	sensitivity = vec2(1.5, 1.5),
	
	connect_address = "127.0.0.1",
	connect_port = 13372,

	server_port = 13372,

	alternative_port = 13373,
	
	nickname = "Daedalus",
	
	tickrate = 60,

	jitter_buffer_ms = 50,
	client_commands_jitter_buffer_ms =150,
	
	simulate_lag = 0,
	packet_loss = 0.00,
	min_latency = 50,
	jitter = 0,
	test_var = 0
}

set_cursor_visible(0)

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
