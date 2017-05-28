#pragma once
#include <string>
#include "game/assets/game_image_id.h"
#include "augs/padding_byte.h"

struct spell_common_data {
	// GEN INTROSPECTOR struct spell_common_data
	bool learned = false;
	pad_bytes<3> pad;
	unsigned personal_electricity_required = 40u;
	unsigned cooldown_ms = 5000u;
	unsigned casting_time_ms = 0u;
	rgba associated_color;
	// END GEN INTROSPECTOR
};

struct spell_appearance {
	// GEN INTROSPECTOR struct spell_appearance
	assets::game_image_id icon = assets::game_image_id::INVALID;
	std::wstring incantation;
	std::wstring name;
	std::wstring description;
	// END GEN INTROSPECTOR
};