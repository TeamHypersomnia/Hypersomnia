#pragma once

struct scoreboard_gui_settings {
	// GEN INTROSPECTOR struct scoreboard_gui_settings
	vec2i player_row_inner_padding = vec2i(2, 4);
	vec2i window_padding = vec2i(4, 8);
	rgba background_color;
	rgba border_color;
	float elements_alpha = 0.8f;

	float bg_lumi_mult = 0.5f;
	float text_lumi_mult = 1.f;

	float text_stroke_lumi_mult = 0.15f;

	float current_player_bg_lumi_mult = 1.7f;
	float current_player_text_lumi_mult = 1.7f;

	float dead_player_bg_lumi_mult = 0.5f;
	float dead_player_text_lumi_mult = 0.8f;

	float dead_player_bg_alpha_mult = 0.3f;
	float dead_player_text_alpha_mult = 0.8f;

	float faction_logo_alpha_mult = 0.8f;
	bool dark_color_overlay_under_score = false;
	// END GEN INTROSPECTOR
};

struct arena_mode_gui_settings {
	// GEN INTROSPECTOR struct arena_mode_gui_settings
	unsigned between_knockout_boxes_pad = 4;
	unsigned inside_knockout_box_pad = 4;
	unsigned weapon_icon_horizontal_pad = 10;
	unsigned show_recent_knockouts_num = 5;
	float keep_recent_knockouts_for_seconds = 8.f;
	unsigned max_weapon_icon_height = 0;

	scoreboard_gui_settings scoreboard_settings;
	rgba money_indicator_color = rgba(249, 133, 255, 255);
	rgba award_indicator_color = yellow;
	vec2i money_indicator_pos = vec2i(-40, 20 + 16 * 4);

	unsigned show_recent_awards_num = 5;
	float keep_recent_awards_for_seconds = 8.f;
	// END GEN INTROSPECTOR
};

