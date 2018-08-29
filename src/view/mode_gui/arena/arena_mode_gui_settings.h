#pragma once

struct scoreboard_gui_settings {
	// GEN INTROSPECTOR struct scoreboard_gui_settings
	vec2i player_box_inner_padding = vec2i(2, 4);
	rgba background_color;
	rgba border_color;
	rgba_channel elements_alpha = 200;
	// END GEN INTROSPECTOR
};

struct arena_mode_gui_settings {
	// GEN INTROSPECTOR struct arena_mode_gui_settings
	unsigned between_knockout_boxes_pad = 4;
	unsigned inside_knockout_box_pad = 4;
	unsigned weapon_icon_horizontal_pad = 10;
	unsigned show_recent_knockouts_num = 5;
	float keep_knockout_boxes_for_seconds = 7.f;
	unsigned max_weapon_icon_height = 0;

	scoreboard_gui_settings scoreboard_settings;
	// END GEN INTROSPECTOR
};

