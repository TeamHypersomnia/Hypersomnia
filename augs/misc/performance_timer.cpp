#include "performance_timer.h"
#include <algorithm>

namespace augs {
	performance_timer::performance_timer() {
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

	void performance_timer::set_count_of_tracked_measurements(size_t count) {
		for (auto& m : meters) {
			m.tracked.resize(count, 0);
		}
	}

	std::wstring performance_timer::sorted_summary() const {
		static thread_local std::vector<measurements> sorted_meters;
		sorted_meters.clear();
		sorted_meters.reserve((int)meter_type::METER_COUNT);

		for (auto& m : meters)
			if (m.was_measured())
				sorted_meters.push_back(m);
		
		std::sort(sorted_meters.begin(), sorted_meters.end());

		std::wstring summary;

		for (auto& m : sorted_meters)
			summary += m.summary();

		return summary;
	}

	void performance_timer::start(meter_type m) {
		meters[(int)m].new_measurement();
	}

	void performance_timer::stop(meter_type m) {
		meters[(int)m].end_measurement();
	}
}