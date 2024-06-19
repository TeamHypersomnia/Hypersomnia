#pragma once

struct explosions_settings {
	// GEN INTROSPECTOR struct explosions_settings
	float sparkle_amount = 1.f;
	float thunder_amount = 1.f;
	float smoke_amount = 1.f;
	// END GEN INTROSPECTOR
	
	bool operator==(const explosions_settings& b) const = default;
};

struct special_effects_settings {
	// GEN INTROSPECTOR struct special_effects_settings
	explosions_settings explosions;
	float particle_stream_amount = 1.f;
	float particle_burst_amount = 1.f;
	// END GEN INTROSPECTOR

	bool operator==(const special_effects_settings& b) const = default;
};
