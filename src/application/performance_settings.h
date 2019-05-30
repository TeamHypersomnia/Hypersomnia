#pragma once

struct performance_settings {
	// GEN INTROSPECTOR struct performance_settings
	int light_calculation_threads = 0;
	// END GEN INTROSPECTOR

	int get_light_calculation_workers() const;
	int get_light_calculation_threads() const;
};
