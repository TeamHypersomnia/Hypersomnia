#pragma once
#include "view/audiovisual_state/special_effects_settings.h"

struct performance_settings {
	// GEN INTROSPECTOR struct performance_settings
	int light_calculation_threads = 0;
	special_effects_settings special_effects;
	// END GEN INTROSPECTOR

	int get_light_calculation_workers() const;
	int get_light_calculation_threads() const;
};
