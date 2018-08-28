#pragma once
#include "augs/misc/enum/enum_array.h"
#include "game/enums/faction_type.h"

struct colors_nha {
	// GEN INTROSPECTOR struct colors_nha
	rgba normal;
	rgba hovered;
	rgba active;
	// END GEN INTROSPECTOR
};

struct faction_color_settings {
	// GEN INTROSPECTOR struct faction_color_settings
	rgba standard;
	rgba current_player_text;

	colors_nha background;
	colors_nha background_dark;
	// END GEN INTROSPECTOR
};

struct faction_view_settings {
	// GEN INTROSPECTOR struct faction_view_settings
	per_faction_t<faction_color_settings> colors;
	// END GEN INTROSPECTOR
};
