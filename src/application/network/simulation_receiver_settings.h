#pragma once

struct simulation_receiver_settings {
	// GEN INTROSPECTOR struct simulation_receiver_settings
	float misprediction_smoothing_multiplier = 0.5f;
	// END GEN INTROSPECTOR

	bool operator==(const simulation_receiver_settings& b) const = default;
};