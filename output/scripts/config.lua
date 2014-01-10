config_table = {
	window_name = "example",
	fullscreen = 0,
	window_border = 0,
	window_x = 0,
	window_y = 0,
	bpp = 24,
	resolution_w = 1500,
	resolution_h = 800,
	doublebuffer = 1,
	
	sensitivity = 2.5
}

if config_table.fullscreen == 1 then
	config_table.resolution_w = get_display().w
	config_table.resolution_h = get_display().h
end