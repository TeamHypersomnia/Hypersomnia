#pragma once
#include <string>

#include "augs/math/vec2.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/timing/timer.h"

namespace augs {
	template <class derived, class T = double>
	class measurements {
	protected:
		std::size_t measurement_index = 0;

		T last_average = static_cast<T>(1);
		T last_minimum = static_cast<T>(1);
		T last_maximum = static_cast<T>(1);
		T last_measurement = static_cast<T>(1);

		bool measured = false;

	public:
		std::string title = "Untitled";
		std::vector<T> tracked;

		measurements(const std::size_t tracked_count = 20u) {
			tracked.resize(tracked_count, 0);
		}

		void measure(const T value) {
			measured = true;
			last_measurement = value;

			tracked[measurement_index] = last_measurement;
			++measurement_index;
			measurement_index %= tracked.size();

			T avg = 0;

			for (auto v : tracked) {
				avg += v;
			}

			last_average = avg / tracked.size();
			last_maximum = maximum_of(tracked);
			last_minimum = minimum_of(tracked);
		}

		std::string summary() const {
			if (!was_measured()) {
				return {};
			}

			const auto& self = *static_cast<const derived*>(this);
			return self.summary_impl();
		}

		bool operator<(const measurements& b) const {
			return get_average_units() < b.get_average_units();
		}

		bool operator>(const measurements& b) const {
			return get_average_units() > b.get_average_units();
		}

		T get_average_units() const {
			return last_average;
		}

		T get_maximum_units() const {
			return last_maximum;
		}

		T get_minimum_units() const {
			return last_minimum;
		}

		T get_last_measurement_units() const {
			return last_measurement;
		}

		bool was_measured() const {
			return measured;
		}
	};

	template <class T>
	class amount_measurements : public measurements<amount_measurements<T>, T> {
		using base = measurements<amount_measurements<T>, T>;
		friend base;

		auto summary_impl() const {
			return typesafe_sprintf("%x: %f2\n", base::title, base::get_average_units());
		}

	public:
		using base::operator<;
		using base::base;
	};

	class time_measurements : public measurements<time_measurements, double> {
		timer tm;

		using base = measurements<time_measurements, double>;
		friend base;

		auto summary_impl() const {
			const auto avg_secs = get_average_units();
			const bool division_by_secs_safe = std::abs(avg_secs) > AUGS_EPSILON<double>;

			if (division_by_secs_safe) {
				return typesafe_sprintf(
					"%x: %f2 ms (%f2 FPS)\n", 
					title,
					avg_secs * 1000,
					1 / avg_secs
				);
			}
			else {
				return typesafe_sprintf(
					"%x: %f2 ms\n", 
					title,
					avg_secs * 1000
				);
			}
		}

		using base::measure;

	public:
		using base::operator<;
		using base::base;

		void start() {
			tm.reset();
		}

		void stop() {
			measure(tm.get<std::chrono::seconds>());
		}
	};
}
