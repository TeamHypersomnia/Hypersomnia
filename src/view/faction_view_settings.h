#pragma once
#include "augs/misc/enum/enum_array.h"
#include "game/enums/faction_type.h"

struct button_colors_nha {
	// GEN INTROSPECTOR struct button_colors_nha
	rgba normal;
	rgba hovered;
	rgba active;
	// END GEN INTROSPECTOR

	bool operator==(const button_colors_nha& b) const = default;
};

struct faction_color_settings {
	// GEN INTROSPECTOR struct faction_color_settings
	rgba standard;
	rgba current_player_text;
	rgba current_player_dead_text;
	rgba player_dead_text;

	button_colors_nha background;
	button_colors_nha background_current;

	rgba background_dark;
	// END GEN INTROSPECTOR

	bool operator==(const faction_color_settings& b) const = default;
};

struct faction_view_settings {
	// GEN INTROSPECTOR struct faction_view_settings
	per_faction_t<faction_color_settings> colors;
	// END GEN INTROSPECTOR

	bool operator==(const faction_view_settings& b) const = default;
};
