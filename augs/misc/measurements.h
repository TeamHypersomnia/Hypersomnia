#pragma once
#include "timer.h"

namespace augs {
	class measurements {
		size_t measurement_index = 0;
		timer tm;

		double last_average;
		double last_minimum;
		double last_maximum;
		double last_measurement;

		bool measured = false;
		bool measurements_are_time = false;
	public:
		measurements(std::wstring title = L"Untitled", bool measurements_are_time = true);

		std::wstring title;

		std::vector<double> tracked;

		std::wstring summary() const;

		bool operator<(const measurements& b) const;

		double get_average_seconds() const;
		double get_maximum_seconds() const;
		double get_minimum_seconds() const;
		double get_last_measurement_seconds() const;

		bool was_measured() const;

		void measure(double);
		void new_measurement();
		void end_measurement();
	};
}
