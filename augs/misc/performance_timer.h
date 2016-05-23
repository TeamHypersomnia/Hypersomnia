#pragma once
#include "measurements.h"
#include <string>

enum class meter_type {
	LOGIC,
	RENDERING,
	CAMERA_QUERY,
	INTERPOLATION,
	VISIBILITY,
	PHYSICS,
	PARTICLES,
	AI,
	PATHFINDING,

	METER_COUNT
};


namespace augs {
	/* WARNING! This variable timestep timer should be replaced with delta accumulation functionality! */
	class performance_timer {
	public:
		measurements meters[(int)meter_type::METER_COUNT];

		void start(meter_type);
		void stop(meter_type);

		performance_timer();

		void set_count_of_tracked_measurements(size_t);
		std::wstring sorted_summary() const;
	};
}