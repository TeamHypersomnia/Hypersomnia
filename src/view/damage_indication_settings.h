#pragma once
#include "augs/templates/maybe.h"

struct damage_indication_settings {
	// GEN INTROSPECTOR struct damage_indication_settings
	augs::maybe<float> numbers_accumulation_speed = augs::maybe<float>::enabled(500.0f);

	rgba critical_color = yellow;
	rgba friendly_damage_border_color = red;

	float indicator_fading_duration_secs = 0.5f;

	float single_indicator_lifetime_secs = 1.f;
	float accumulative_indicator_idle_lifetime_secs = 1.f;

	int indicator_rising_speed = 50;

	int small_damage_threshold = 25;
	int medium_damage_threshold = 61;

	vec2 accumulative_indicator_offset = vec2(0, -100);
	std::vector<vec2> single_indicator_offsets;

	float white_damage_highlight_secs = 0.11f;
	float character_silhouette_damage_highlight_secs = 0.11f;
	// END GEN INTROSPECTOR
};
