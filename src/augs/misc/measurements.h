#pragma once
#include <string>

#include "augs/math/vec2.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/timing/timer.h"
#include "augs/misc/scope_guard.h"

namespace augs {
	template <class derived, class T = double>
	class measurements {
	protected:
		std::size_t measurement_index = 0;

		T last_average = T();
		T last_minimum = T();
		T last_maximum = T();
		T last_measurement = T();

		bool measured = false;

	public:
		std::string title = "Untitled";
		std::vector<T> tracked;

		measurements(const std::size_t tracked_count = 20u) {
			tracked.resize(tracked_count);
		}

		void measure(const T value) {
			measured = true;
			last_measurement = value;

			tracked[measurement_index] = last_measurement;
			++measurement_index;
			measurement_index %= tracked.size();

			T avg = T();

			for (auto v : tracked) {
				avg += v;
			}

			last_average = avg / static_cast<unsigned>(tracked.size());
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

	public:
		using base::operator<;
		using base::base;
		using base::measure;

		void start() {
			tm.reset();
		}

		void stop() {
			measure(tm.get<std::chrono::seconds>());
		}
	};

	template <class T, class = void>
	struct has_title : std::false_type {};

	template <class T>
	struct has_title<T, decltype(std::declval<T>().title, void())> : std::true_type {};

	template <class T>
	constexpr bool has_title_v = has_title<T>::value;

	static_assert(has_title_v<time_measurements>);
	static_assert(has_title_v<amount_measurements<std::size_t>>);
}


inline auto add_scope_duration(double& total) {
	augs::timer tm;
	return augs::scope_guard([tm, &total](){ total += tm.get<std::chrono::seconds>(); });
}

struct additive_time_scope {
	double total = 0.0;
	augs::time_measurements& into;

	additive_time_scope(additive_time_scope&&) = delete;
	additive_time_scope(const additive_time_scope& b) = delete;

	~additive_time_scope() {
		into.measure(total);
	}
};

inline auto measure_scope(augs::time_measurements& m) {
	m.start();
	return augs::scope_guard([&m]() { m.stop(); });
}

inline auto measure_scope(additive_time_scope& m) {
	augs::timer tm;
	return augs::scope_guard([tm, &m](){ m.total += tm.get<std::chrono::seconds>(); });
}

inline auto measure_scope_additive(augs::time_measurements& into) {
	return additive_time_scope { 0.0, into };
}
