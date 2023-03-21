#pragma once
#include "augs/graphics/rgba.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_game_mode_resource;

struct editor_playtesting_settings {
	// GEN INTROSPECTOR struct editor_playtesting_settings
	editor_typed_resource_id<editor_game_mode_resource> mode;

	bool skip_warmup = true;
	bool skip_freeze_time = true;
	bool unlimited_money = true;
	// END GEN INTROSPECTOR
};
