#pragma once
#include <string>
#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"

struct spell_logical_meta {
	// GEN INTROSPECTOR struct spell_logical_meta
	unsigned personal_electricity_required = 40u;
	unsigned cooldown_ms = 5000u;
	unsigned casting_time_ms = 0u;
	unsigned perk_duration_seconds = 0u;

	rgba border_col;
	// END GEN INTROSPECTOR
};

struct spell_data {
	// GEN INTROSPECTOR struct spell_data
	spell_logical_meta logical;

	assets::game_image_id icon = assets::game_image_id::COUNT;
	augs::constant_size_wstring<INCANTATION_STRING_LENGTH> incantation;
	std::wstring spell_name;
	std::wstring spell_description;
	// END GEN INTROSPECTOR

	spell_logical_meta get_logical_meta() const {
		return logical;
	}
};