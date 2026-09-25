#pragma once
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "game/balance_params.h"
#include "application/setups/editor/resources/editor_sound_effect.h"

struct editor_arena_settings {
	// GEN INTROSPECTOR struct editor_arena_settings
	editor_typed_resource_id<editor_game_mode_resource> default_server_mode;
	bool include_disabled_nodes = true;
	float default_zoom = BALANCE_ZOOM_OUT;
	bool minimap_tab_shows_all_islands = false;
	rgba ambient_light_color = rgba(53, 97, 102, 255);
	vec2 shadow_step = vec2(1.87f, 2.34f);
	real32 shadow_strength = 0.35f;
	real32 shadow_hue_preservation = 1.0f;
	real32 shadow_smoothness = 1.0f;
	editor_theme warmup_theme;
	uint32_t navmesh_cell_size = 128;
	rgba debug_navmesh_free_color = rgba(0, 255, 255, 60);
	rgba debug_navmesh_occupied_color = rgba(255, 0, 0, 60);
	rgba debug_navmesh_portal_color = rgba(0, 255, 0, 60);
	rgba debug_navigation_color = rgba(0, 255, 0, 220);
	// END GEN INTROSPECTOR

	bool operator==(const editor_arena_settings&) const = default;
};
