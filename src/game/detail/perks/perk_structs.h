#pragma once
#include "game/detail/perks/perk_timing.h"

struct perk_appearance {
	// GEN INTROSPECTOR struct perk_appearance
	assets::image_id icon;
	entity_name_str description;
	rgba bar_color;
	// END GEN INTROSPECTOR
	
	auto get_icon() const {
		return icon;
	}

	auto get_bar_color() const {
		return bar_color;
	}

	const auto& get_description() const {
		return description;
	}
};