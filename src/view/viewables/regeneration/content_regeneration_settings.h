#pragma once

struct content_regeneration_settings {
	// GEN INTROSPECTOR struct content_regeneration_settings
	bool regenerate_every_time = false;

	unsigned atlas_blitting_threads = 2;
	unsigned neon_regeneration_threads = 2;
	// END GEN INTROSPECTOR
};