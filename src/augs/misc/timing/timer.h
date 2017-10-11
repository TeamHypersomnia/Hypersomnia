#pragma once
#include <ratio>
#include <chrono>

#include "augs/misc/timing/delta.h"

namespace augs {
	class timer {
		std::chrono::high_resolution_clock::time_point ticks;

		static auto now() {
			return std::chrono::high_resolution_clock::now();
		}

	public:
		timer();

		template <class resolution>
		double get() const {
			using namespace std::chrono;
			return duration_cast<duration<double, typename resolution::period>>(now() - ticks).count();
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
