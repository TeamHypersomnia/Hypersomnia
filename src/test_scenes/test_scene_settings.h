#pragma once

struct test_scene_settings {
	// GEN INTROSPECTOR struct test_scene_settings
	bool create_minimal = false;
	unsigned scene_tickrate = 60;
	// END GEN INTROSPECTOR

	bool operator==(const test_scene_settings& b) const = default;
};
