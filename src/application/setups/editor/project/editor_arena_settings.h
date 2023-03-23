#pragma once
#include "augs/graphics/rgba.h"
#include "application/setups/editor/resources/editor_sound_effect.h"

struct editor_arena_settings {
	// GEN INTROSPECTOR struct editor_arena_settings
	editor_typed_resource_id<editor_game_mode_resource> default_server_mode;
	rgba ambient_light_color = rgba(53, 97, 102, 255);
	editor_theme warmup_theme;
	// END GEN INTROSPECTOR
};
