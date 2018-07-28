#pragma once
#include <ratio>
#include <chrono>

#include "augs/misc/timing/delta.h"

namespace augs {
	class timer {
		// GEN INTROSPECTOR class augs::timer
		std::chrono::high_resolution_clock::time_point ticks;
		// END GEN INTROSPECTOR

	public:
		timer();

		template <class resolution>
		double get() const {
			using namespace std::chrono;
			const auto now = std::chrono::high_resolution_clock::now();
			return duration_cast<duration<double, typename resolution::period>>(now - ticks).count();
		}

		void reset();

		template <class resolution>
		auto extract() {
			const auto amount = get<resolution>();
			reset();
			return amount;
		}

		auto extract_delta() {
			return augs::delta{ extract<std::chrono::seconds>() };
		}
	};
}
