#pragma once

struct per_character_input_settings {
	// GEN INTROSPECTOR struct per_character_input_settings
	vec2 crosshair_sensitivity = vec2(1000.f, 1000.f);
	bool forward_moves_towards_crosshair = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	bool operator==(const per_character_input_settings& b) const {
		return 
			crosshair_sensitivity == b.crosshair_sensitivity 
			&& forward_moves_towards_crosshair == b.forward_moves_towards_crosshair
		;
	}

	bool operator!=(const per_character_input_settings& b) const {
		return !operator==(b);
	}
};
