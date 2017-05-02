#include "cosmic_profiler.h"
#include "augs/templates/container_templates.h"
#include <algorithm>

cosmic_profiler::cosmic_profiler() {
	meters[(int)meter_type::GUI].title = L"GUI";
	meters[(int)meter_type::LOGIC].title = L"Logic";
	meters[(int)meter_type::RENDERING].title = L"Rendering";
	meters[(int)meter_type::CAMERA_QUERY].title = L"Camera query";
	meters[(int)meter_type::INTERPOLATION].title = L"Interpolation";
	meters[(int)meter_type::PHYSICS].title = L"Physics";
	meters[(int)meter_type::VISIBILITY].title = L"Visibility";
	meters[(int)meter_type::AI].title = L"AI";
	meters[(int)meter_type::PATHFINDING].title = L"Pathfinding";
	meters[(int)meter_type::PARTICLES].title = L"Particles";

	set_count_of_tracked_measurements(20);
}

void cosmic_profiler::set_count_of_tracked_measurements(size_t count) {
	for (auto& m : meters) {
		m.tracked.resize(count, 0);
	}
}

std::wstring cosmic_profiler::sorted_summary(const bool detailed) const {
	std::vector<augs::measurements> sorted_meters;

	for (auto& m : meters)
		if (m.was_measured())
			sorted_meters.push_back(m);

	sort_container(sorted_meters);
	std::reverse(sorted_meters.begin(), sorted_meters.end());

	std::wstring summary = raycasts.summary();

	summary += entropy_length.summary();

	for (auto& m : sorted_meters)
		summary += m.summary();

	summary += complete_reinference.summary();
	summary += delta_encoding.summary();
	summary += delta_decoding.summary();
	
	summary += total_save.summary();
	summary += size_calculation_pass.summary();
	summary += memory_allocation_pass.summary();
	summary += serialization_pass.summary();
	summary += writing_savefile.summary();

	summary += total_load.summary();
	summary += reading_savefile.summary();
	summary += deserialization_pass.summary();

	summary += duplication.summary();

	summary += delta_bytes.summary();

	return summary;
}

void cosmic_profiler::start(meter_type m) {
	meters[(int)m].new_measurement();
}

void cosmic_profiler::stop(meter_type m) {
	meters[(int)m].end_measurement();
}