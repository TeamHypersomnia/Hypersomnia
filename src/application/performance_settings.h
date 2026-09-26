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

enum class shadow_quality_type {
	// GEN INTROSPECTOR enum class shadow_quality_type
	NONE,
	LOW,
	NORMAL,
	COUNT
	// END GEN INTROSPECTOR
};

/*
	Multipliers scale what the map sets.
*/

struct sun_shadow_preferences {
	// GEN INTROSPECTOR struct sun_shadow_preferences
	shadow_quality_type quality = shadow_quality_type::NORMAL;
	bool foreground = true;
	bool background = true;
	float strength_mult = 1.0f;
	float smoothness_mult = 1.0f;
	// END GEN INTROSPECTOR

	bool operator==(const sun_shadow_preferences&) const = default;
};

struct point_light_shadow_preferences {
	// GEN INTROSPECTOR struct point_light_shadow_preferences
	bool soft = true;
	float smoothness_mult = 1.0f;
	bool heights = true;
	// END GEN INTROSPECTOR

	bool operator==(const point_light_shadow_preferences&) const = default;
};

struct performance_settings {
	// GEN INTROSPECTOR struct performance_settings
	special_effects_settings special_effects;
	int max_particles_in_single_job = 2500;
	augs::maybe<int> custom_num_pool_workers = augs::maybe<int>(0, false);
	accuracy_type wall_light_drawing_precision = accuracy_type::EXACT;
	sun_shadow_preferences sun_shadows;
	point_light_shadow_preferences point_light_shadows;
	bool posterize_neons = true;
	swap_buffers_moment swap_window_buffers_when = swap_buffers_moment::AFTER_HELPING_LOGIC_THREAD;
	// END GEN INTROSPECTOR

	bool operator==(const performance_settings& b) const = default;

	int get_num_pool_workers() const;
	static int get_default_num_pool_workers();
};
