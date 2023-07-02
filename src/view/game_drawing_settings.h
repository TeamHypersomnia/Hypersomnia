#pragma once
#include "augs/pad_bytes.h"
#include "augs/templates/maybe.h"
#include "augs/graphics/rgba.h"
#include "game/modes/detail/fog_of_war_settings.h"
#include "view/character_hud_type.h"

struct fog_of_war_appearance_settings {
	// GEN INTROSPECTOR struct fog_of_war_appearance_settings
	bool overlay_color_on_visible = true;
	rgba overlay_color = rgba(255, 255, 255, 2);
	// END GEN INTROSPECTOR
};

enum class offscreen_reference_type {
	// GEN INTROSPECTOR enum class offscreen_reference_type
	SCREEN_CENTER,
	CHARACTER_POSITION,
	COUNT
	// END GEN INTROSPECTOR
};

struct crosshair_drawing_settings {
	// GEN INTROSPECTOR struct crosshair_drawing_settings
	int scale = 3;
	int border_width = 1;
	bool show_dot = true;
	float dot_size = 1.0f;
	float segment_length = 10.0f;
	float recoil_expansion_base = 5.0f;
	float recoil_expansion_mult = 1.0f;
	rgba inside_color = white;
	rgba border_color = black;
	// END GEN INTROSPECTOR
};

struct game_drawing_settings {
	// GEN INTROSPECTOR struct game_drawing_settings
	bool auto_zoom = false;
	float custom_zoom = 1.0f;

	bool draw_crosshairs = true;
	bool draw_weapon_laser = true;
	bool draw_aabb_highlighter = true;
	bool draw_inventory = true;
	bool draw_hotbar = true;
	augs::maybe<float> draw_area_markers = augs::maybe<float>(0.5f, true);
	augs::maybe<float> draw_callout_indicators = augs::maybe<float>(0.5f, true);
	character_hud_type enemy_hud_mode = character_hud_type::SMALL_HEALTH_BAR;
	bool draw_hp_bar = true;
	bool draw_cp_bar = true;
	bool draw_pe_bar = false;
	bool draw_character_status = true;
	bool draw_remaining_ammo = true;

	bool draw_offscreen_indicators = true;
	bool draw_offscreen_callouts = true;
	bool draw_nicknames = true;
	bool draw_small_health_bars = true;
	bool draw_health_numbers = true;
	bool draw_damage_indicators = false;

	bool occlude_neons_under_sentiences = true;

	bool cinematic_mode = false;


	offscreen_reference_type offscreen_reference_mode = offscreen_reference_type::CHARACTER_POSITION;

	augs::maybe<float> draw_teammate_indicators = augs::maybe<float>(1.f, true);
	augs::maybe<rgba> draw_danger_indicators = augs::maybe<rgba>(rgba(255, 50, 50, 255), true);
	augs::maybe<float> draw_tactical_indicators = augs::maybe<float>(1.f, true);

	float show_danger_indicator_for_seconds = 5.f;
	float fade_danger_indicator_for_seconds = 2.f;

	float show_death_indicator_for_seconds = 5.f;
	float fade_death_indicator_for_seconds = 2.f;

	vec2 radar_pos = vec2i(-40, 20 + 16 * 4);
	bool print_current_character_callout = false;
	int nickname_characters_for_offscreen_indicators = 4;

	fog_of_war_settings fog_of_war;
	fog_of_war_appearance_settings fog_of_war_appearance;
	crosshair_drawing_settings crosshair;
	// END GEN INTROSPECTOR
};