#pragma once
#include "misc/performance_timer.h"
#include "misc/measurements.h"

class cosmic_profiler {
public:
	augs::measurements raycasts = augs::measurements(L"Raycasts", false);
	augs::measurements triangles = augs::measurements(L"Triangles", false);
	augs::measurements fps_counter = augs::measurements(L"Frame");

	augs::performance_timer performance;

	std::wstring summary(bool detailed) const;
};