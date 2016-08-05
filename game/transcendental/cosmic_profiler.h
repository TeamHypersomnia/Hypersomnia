#pragma once
#include "augs/misc/measurements.h"

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
	augs::measurements complete_resubstantiation = augs::measurements(L"Complete resubstantiation", true, 1);

	augs::measurements raycasts = augs::measurements(L"Raycasts", false);
	augs::measurements entropy_length = augs::measurements(L"Entropy length", false);
	augs::measurements meters[(int)meter_type::METER_COUNT];

	mutable augs::measurements delta_encoding = augs::measurements(L"Delta encoding");
	mutable augs::measurements delta_decoding = augs::measurements(L"Delta decoding");

	void start(meter_type);
	void stop(meter_type);

	cosmic_profiler();

	void set_count_of_tracked_measurements(size_t);
	std::wstring sorted_summary() const;
};