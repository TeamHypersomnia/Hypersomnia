#pragma once

struct per_character_input_settings {
	// GEN INTROSPECTOR struct per_character_input_settings
	vec2 crosshair_sensitivity = vec2(3.f, 3.f);
	bool keep_movement_forces_relative_to_crosshair = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	bool operator==(const per_character_input_settings& b) const {
		return 
			crosshair_sensitivity == b.crosshair_sensitivity 
			&& keep_movement_forces_relative_to_crosshair == b.keep_movement_forces_relative_to_crosshair
		;
	}

	bool operator!=(const per_character_input_settings& b) const {
		return !operator==(b);
	}
};
