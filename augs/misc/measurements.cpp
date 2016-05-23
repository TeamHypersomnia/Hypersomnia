#include "measurements.h"
#include <algorithm>
#include "log.h"

namespace augs {
	measurements::measurements(std::wstring title, bool measurements_are_time) : title(title), measurements_are_time(measurements_are_time) {
		tracked.resize(20, 0);
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
		last_maximum = *std::max_element(tracked.begin(), tracked.end());
		last_minimum = *std::min_element(tracked.begin(), tracked.end());
	}

	void measurements::end_measurement() {
		measure(tm.get<std::chrono::seconds>());
	}

	std::wstring measurements::summary() const {
		double scale = 1000;
		bool detailed = false;

		if(detailed)
		return typesafe_sprintf(L"%x: %f (%F) (avg: %f min: %f max: %f)\n", title, 
			get_last_measurement_seconds()*scale,
			get_average_seconds() *scale,
			get_minimum_seconds() * scale,
			get_maximum_seconds() * scale);
		else {
			scale = 1000;
			if (measurements_are_time) {
				return typesafe_sprintf(L"%x: %f2 ms (%f2 FPS)\n", title,
					get_average_seconds() * scale,
					1 / get_average_seconds());
			}
			else
				return typesafe_sprintf(L"%x: %f2\n", title, get_average_seconds());
		}
	}

	double measurements::get_average_seconds() const {
		return last_average;
	}

	double measurements::get_maximum_seconds() const {
		return last_maximum;
	}

	double measurements::get_minimum_seconds() const {
		return last_minimum;
	}

	double measurements::get_last_measurement_seconds() const {
		return last_measurement;
	}

	bool measurements::was_measured() const {
		return measured;
	}

	void measurements::new_measurement() {
		tm.reset();
	}
}