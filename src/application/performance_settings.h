#pragma once
#include "view/audiovisual_state/special_effects_settings.h"
#include "augs/templates/maybe.h"

struct performance_settings {
	// GEN INTROSPECTOR struct performance_settings
	special_effects_settings special_effects;
	int max_particles_in_single_job = 2500;
	augs::maybe<int> custom_num_pool_workers = augs::maybe<int>(0, false);
	// END GEN INTROSPECTOR

	int get_num_pool_workers() const;
	static int get_default_num_pool_workers();
};
