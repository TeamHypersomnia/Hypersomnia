#pragma once
#include "augs/misc/enum/enum_array.h"
#include "game/enums/faction_type.h"

struct button_colors_nha {
	// GEN INTROSPECTOR struct button_colors_nha
	rgba normal = white;
	rgba hovered = white;
	rgba active = white;
	// END GEN INTROSPECTOR

	bool operator==(const button_colors_nha& b) const = default;
};

struct faction_color_settings {
	// GEN INTROSPECTOR struct faction_color_settings
	rgba standard = white;
	rgba current_player_text = white;
	rgba current_player_dead_text = white;
	rgba player_dead_text = white;

	button_colors_nha background;
	button_colors_nha background_current;

	rgba background_dark = black;
	// END GEN INTROSPECTOR

	bool operator==(const faction_color_settings& b) const = default;
};

struct faction_view_settings {
	// GEN INTROSPECTOR struct faction_view_settings
	per_faction_t<faction_color_settings> colors;
	// END GEN INTROSPECTOR

	bool operator==(const faction_view_settings& b) const = default;
};
