#pragma once
#include "augs/graphics/rgba.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

#include "game/enums/faction_type.h"

struct editor_game_mode_resource;

struct editor_playtesting_settings {
	// GEN INTROSPECTOR struct editor_playtesting_settings
	editor_typed_resource_id<editor_game_mode_resource> mode;

	faction_type starting_faction = faction_type::METROPOLIS;

	bool skip_warmup = true;
	bool skip_freeze_time = true;
	bool unlimited_money = true;
	bool spawn_bots = false;
	bool see_enemies_behind_walls = false;
	// END GEN INTROSPECTOR

	bool operator==(const editor_playtesting_settings&) const = default;
};
