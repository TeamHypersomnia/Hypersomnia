#include "measurements.h"
#include <algorithm>
#include "augs/log.h"
#include "augs/math/vec2.h"
#include "augs/templates/container_templates.h"

namespace augs {
	measurements::measurements(std::wstring title, bool measurements_are_time, size_t tracked_count) : title(title), measurements_are_time(measurements_are_time) {
		last_average = 1.0;
		last_maximum = 1.0;
		last_minimum = 1.0;
		last_measurement = 1.0;
		tracked.resize(tracked_count, 0);
	}

	void measurements::measure(double value) {
		measured = true;
		last_measurement = value;

		tracked[measurement_index] = last_measurement;
		++measurement_index;
		measurement_index %= tracked.size();

		double avg = 0.;

		for (auto v : tracked)
			avg += v;

		last_average = avg / tracked.size();
		last_maximum = maximum_of(tracked);
		last_minimum = minimum_of(tracked);
	}

	void measurements::end_measurement() {
		measure(tm.get<std::chrono::seconds>());
	}

	std::wstring measurements::summary() const {
		if (!was_measured())
			return std::wstring();

		double scale = 1000;
		bool detailed = false;

		auto avg_secs = get_average_units();

		if(detailed)
		return typesafe_sprintf(L"%x: %f (%F) (avg: %f min: %f max: %f)\n", title, 
			get_last_measurement_units()*scale,
			avg_secs * scale,
			get_minimum_units() * scale,
			get_maximum_units() * scale);
		else {
			scale = 1000;
			if (measurements_are_time) {
				if (std::abs(avg_secs) > AUGS_EPSILON<double>) {
					return typesafe_sprintf(L"%x: %f2 ms (%f2 FPS)\n", title,
						avg_secs * scale,
						1 / avg_secs);
				}
				else
					return typesafe_sprintf(L"%x: %f2 ms\n", title,
						avg_secs * scale);
			}
			else
				return typesafe_sprintf(L"%x: %f2\n", title, avg_secs);
		}
	}

	bool measurements::operator<(const measurements& b) const {
		return get_average_units() < b.get_average_units();
	}

	double measurements::get_average_units() const {
		return last_average;
	}

	double measurements::get_maximum_units() const {
		return last_maximum;
	}

	double measurements::get_minimum_units() const {
		return last_minimum;
	}

	double measurements::get_last_measurement_units() const {
		return last_measurement;
	}

	bool measurements::was_measured() const {
		return measured;
	}

	void measurements::new_measurement() {
		tm.reset();
	}
}