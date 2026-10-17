#pragma once

struct per_character_input_settings {
	// GEN INTROSPECTOR struct per_character_input_settings
	vec2 crosshair_sensitivity = vec2(1000.f, 1000.f);
	bool forward_moves_towards_crosshair = false;
	bool pickup_swaps_wielded_firearm = true;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR

	bool operator==(const per_character_input_settings& b) const = default;
};
