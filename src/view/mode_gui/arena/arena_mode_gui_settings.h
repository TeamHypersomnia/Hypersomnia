#pragma once

struct buy_menu_gui_settings {
	// GEN INTROSPECTOR struct buy_menu_gui_settings
	rgba disabled_bg = rgba(170, 0, 0, 90);
	rgba disabled_active_bg = rgba(180, 0, 0, 150);

	rgba already_owns_bg = rgba(0, 170, 0, 90);
	rgba already_owns_active_bg = rgba(0, 180, 0, 150);

	rgba already_owns_other_type_bg = rgba(0, 170, 0, 90);
	rgba already_owns_other_type_active_bg = rgba(0, 180, 0, 150);
	// END GEN INTROSPECTOR
};

struct scoreboard_gui_settings {
	// GEN INTROSPECTOR struct scoreboard_gui_settings
	vec2i player_row_inner_padding = vec2i(2, 4);
	vec2i window_padding = vec2i(4, 8);
	rgba background_color = black;
	rgba border_color = white;
	float cell_bg_alpha = 0.8f;

	float bg_lumi_mult = 0.5f;
	float text_lumi_mult = 1.f;

	float text_stroke_lumi_mult = 0.15f;

	float current_player_bg_lumi_mult = 1.7f;
	float current_player_text_lumi_mult = 1.7f;

	float dead_player_bg_lumi_mult = 0.5f;
	float dead_player_text_lumi_mult = 0.8f;

	float dead_player_bg_alpha_mult = 0.3f;
	float dead_player_text_alpha_mult = 0.8f;

	float faction_logo_alpha = 0.8f;
	float icon_alpha = 0.8f;
	float avatar_alpha = 1.f;
	bool dark_color_overlay_under_score = false;
	// END GEN INTROSPECTOR
};

struct arena_context_tip_settings {
	// GEN INTROSPECTOR struct arena_context_tip_settings
	float tip_offset_mult = 0.7f;
	rgba tip_text_color = white;
	rgba bound_key_color = yellow;
	rgba item_name_color = yellow;
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
	buy_menu_gui_settings buy_menu_settings;
	rgba money_indicator_color = rgba(249, 133, 255, 255);
	rgba award_indicator_color = yellow;
	vec2i money_indicator_pos = vec2i(-40, 20 + 16 * 4);

	unsigned show_recent_awards_num = 5;
	float keep_recent_awards_for_seconds = 8.f;

	bool show_client_resyncing_notifier = true;

	float death_summary_offset_mult = 0.7f;

	augs::maybe<arena_context_tip_settings> context_tip_settings;

	bool show_spectator_overlay = true;
	// END GEN INTROSPECTOR
};

