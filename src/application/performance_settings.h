#pragma once
#include "view/audiovisual_state/special_effects_settings.h"
#include "augs/templates/maybe.h"
#include "augs/enums/accuracy_type.h"

enum class swap_buffers_moment {
	// GEN INTROSPECTOR enum class swap_buffers_moment
	AFTER_HELPING_LOGIC_THREAD,
	AFTER_GL_COMMANDS,
	COUNT
	// END GEN INTROSPECTOR
};

struct performance_settings {
	// GEN INTROSPECTOR struct performance_settings
	special_effects_settings special_effects;
	int max_particles_in_single_job = 2500;
	augs::maybe<int> custom_num_pool_workers = augs::maybe<int>(0, false);
	accuracy_type wall_light_drawing_precision = accuracy_type::EXACT;
	swap_buffers_moment swap_window_buffers_when = swap_buffers_moment::AFTER_HELPING_LOGIC_THREAD;
	// END GEN INTROSPECTOR

	bool operator==(const performance_settings& b) const = default;

	int get_num_pool_workers() const;
	static int get_default_num_pool_workers();
};
