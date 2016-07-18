#pragma once
#include "misc/measurements.h"

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

class cosmic_profiler {
public:
	augs::measurements raycasts = augs::measurements(L"Raycasts", false);
	augs::measurements meters[(int)meter_type::METER_COUNT];

	void start(meter_type);
	void stop(meter_type);

	cosmic_profiler();

	void set_count_of_tracked_measurements(size_t);
	std::wstring sorted_summary() const;
};