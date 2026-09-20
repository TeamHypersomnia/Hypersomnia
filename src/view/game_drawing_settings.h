#pragma once
#include <algorithm>
#include <memory>
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

	bool operator==(const fog_of_war_appearance_settings& b) const = default;
};

enum class offscreen_reference_type {
	// GEN INTROSPECTOR enum class offscreen_reference_type
	SCREEN_CENTER,
	CHARACTER_POSITION,
	COUNT
	// END GEN INTROSPECTOR
};

enum class crosshair_type {
	// GEN INTROSPECTOR enum class crosshair_type
	CLASSIC,
	CIRCULAR,
	COUNT
	// END GEN INTROSPECTOR
};

struct crosshair_drawing_settings {
	// GEN INTROSPECTOR struct crosshair_drawing_settings
	crosshair_type type = crosshair_type::CLASSIC;
	int scale = 3;
	int border_width = 1;
	bool show_dot = true;
	float dot_size = 1.0f;
	float segment_length = 10.0f;
	float segment_thickness = 1.0f;
	float recoil_expansion_base = 5.0f;
	float recoil_expansion_mult = 1.0f;
	rgba inside_color = white;
	rgba border_color = black;
	rgba background_color = rgba(0, 0, 0, 50);
	// END GEN INTROSPECTOR

	bool operator==(const crosshair_drawing_settings& b) const = default;
};

enum class hud_corner_type {
	// GEN INTROSPECTOR enum class hud_corner_type
	LEFT_TOP,
	RIGHT_TOP,
	LEFT_BOTTOM,
	RIGHT_BOTTOM,
	COUNT
	// END GEN INTROSPECTOR
};

enum class minimap_tab_behavior_type {
	// GEN INTROSPECTOR enum class minimap_tab_behavior_type
	SHOW_ENTIRE_MAP,
	ZOOM_OUT,
	NONE,
	COUNT
	// END GEN INTROSPECTOR
};

struct minimap_settings {
	// GEN INTROSPECTOR struct minimap_settings
	bool enabled = true;
	float master_alpha = 1.0f;
	float master_alpha_under_tab = 1.0f;
	float background_alpha = 1.0f;
	float background_alpha_under_tab = 1.0f;
	hud_corner_type position = hud_corner_type::RIGHT_BOTTOM;
	int size = 300;
	int size_under_tab = 300;
	int border_thickness = 1;
	float dot_size_mult = 1.5f;
	float range_mult = 1.5f;
	float show_entire_map_if_fits_mult = 2.0f;
	minimap_tab_behavior_type tab_behavior = minimap_tab_behavior_type::SHOW_ENTIRE_MAP;
	float scoreboard_range_mult = 2.0f;
	rgba background_color = rgba(0, 44, 0, 255);
	rgba border_color = rgba(0, 255, 90, 255);
	rgba obstacle_color = rgba(0, 220, 78, 255);
	rgba portal_color = cyan;
	rgba hazard_color = rgba(160, 0, 0, 120);
	rgba marker_color = cyan;
	rgba fog_of_war_color = rgba(255, 255, 255, 0);
	rgba player_color = white;
	rgba teammate_color = yellow;
	rgba enemy_color = red;
	bool animate_laser_dashes = false;
	bool clamp_important_to_border = true;
	bool only_under_tab = false;
	bool draw_viewed_player_ring = true;
	// END GEN INTROSPECTOR

	bool operator==(const minimap_settings& b) const = default;

	/*
		Deliberately outside the introspector so they cannot be set from
		config files - these are filled at runtime by setups that draw
		custom HUD around the minimap (e.g. the tutorial's progress bars).

		extra_bottom_margin pushes the minimap up when it sits in a bottom
		corner, to make room for HUD drawn at the very bottom of the screen.

		extra_hud_space is the height of the HUD block drawn right at the
		minimap's edge (e.g. the tutorial's stage bar) - other elements
		sharing the corner have to make room for it as well.
	*/
	int extra_bottom_margin = 0;
	int extra_hud_space = 0;

	/*
		The minimap can be kept more transparent (or hidden altogether)
		while the scoreboard is closed, so that it obscures less of the
		gameplay area, and become fully opaque once the player holds TAB.
	*/
	/*
		The minimap can grow once the scoreboard is held,
		so that it stays small during the play but becomes
		properly readable when actually looked at.

		Note that only the gameplay size is ever accounted for
		by the HUD elements sharing the minimap's corner -
		the enlarged one is transient, and the scoreboard
		covers most of the screen anyway.
	*/
	int calc_size(const bool under_tab) const {
		return under_tab ? size_under_tab : size;
	}

	float calc_master_alpha(const bool under_tab) const {
		return std::clamp(under_tab ? master_alpha_under_tab : master_alpha, 0.0f, 1.0f);
	}

	/*
		The background alpha is a separate multiplier on top of the master one,
		applied only to what is not a gameplay indicator: the background fill,
		the border, the obstacle geometry, the portals and the hazards.

		This way the map itself can be faded out to a bare hint
		while the players, the markers and the bomb stay fully readable.
	*/
	float calc_background_alpha(const bool under_tab) const {
		return std::clamp(under_tab ? background_alpha_under_tab : background_alpha, 0.0f, 1.0f);
	}

	minimap_settings with_faded_background(const bool under_tab) const {
		auto result = *this;
		const auto alpha = calc_background_alpha(under_tab);

		if (alpha < 1.0f) {
			const auto faded = {
				std::addressof(result.background_color),
				std::addressof(result.border_color),
				std::addressof(result.obstacle_color),
				std::addressof(result.portal_color),
				std::addressof(result.hazard_color)
			};

			for (const auto c : faded) {
				c->mult_alpha(alpha);
			}
		}

		return result;
	}

	bool is_visible(const bool under_tab) const {
		if (!enabled) {
			return false;
		}

		if (only_under_tab && !under_tab) {
			return false;
		}

		return calc_master_alpha(under_tab) > 0.0f;
	}

	/*
		Whether the minimap permanently occupies the given corner,
		so the other HUD elements have to make room for it there.
	*/
	bool occupies_corner(const hud_corner_type corner) const {
		return is_visible(false) && position == corner;
	}
};

struct game_drawing_settings {
	// GEN INTROSPECTOR struct game_drawing_settings
	bool snap_zoom_to_fov_size = true;
	int snap_zoom_to_multiple_if_different_by_pixels = 10;
	float custom_zoom = 1.0f;

	bool draw_enemy_silhouettes_in_spectator = true;

	bool draw_bullet_shadows = true;
	bool draw_bullet_trails = true;
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

	bool draw_value_on_aura_bar = true;
	bool aura_bar_value_as_percent = false;
	int aura_bar_percent_decimal_places = 1;

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

	fog_of_war_appearance_settings fog_of_war_appearance;
	crosshair_drawing_settings crosshair;
	minimap_settings minimap;

	bool teammates_are_enemies = false;
	bool stencil_before_light_pass = false;
	// END GEN INTROSPECTOR

	/*
		Deliberately outside the introspector so it cannot be set from config
		files - players must not be able to easily zoom the camera out.
		When > 0, sets the camera zoom exactly to this multiplier, bypassing
		custom_zoom. Used by the main menu for the exact 3x effect.
	*/
	float _override_zoom = 0.0f;

	/*
		Not introspected: never read from the player's config. It is filled at
		runtime from the active mode ruleset (see arena_handle::adjust), and
		defaults to the balance constants when no mode is adjusting it.
	*/
	fog_of_war_settings fog_of_war;

	bool operator==(const game_drawing_settings& b) const = default;
};