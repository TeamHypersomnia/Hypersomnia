#include "cosmic_profiler.h"

std::wstring cosmic_profiler::summary(bool detailed) const {
	auto result = fps_counter.summary();

	if (detailed)
		result += performance.sorted_summary();

	result = triangles.summary() + result;

	if (detailed)
		result = raycasts.summary() + result;

	return result;
}