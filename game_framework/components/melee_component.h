#pragma once
#pragma once

#include "../globals/filters.h"

namespace components {
	struct melee {
		bool inverse_swing_direction = false;

		float swing_duration = 0.f;
		float swing_radius = 0.f;
		float swing_angle = 0.f;
		float swing_angular_offset = 0.f;
		float swing_interval_ms = 500.f;
		int query_vertices = 7;

		b2Filter melee_filter;
		b2Filter melee_obstruction_filter;
	};

}