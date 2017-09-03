#pragma once
#include <ratio>
#include <chrono>

namespace augs {
	class timer {
		std::chrono::high_resolution_clock::time_point ticks;
		std::chrono::duration<std::chrono::system_clock::rep, std::chrono::system_clock::period> paused_difference;

		bool is_paused = false;

	public:
		timer();
		
		void pause(const bool flag);
		void reset();

		/* returns time that has lasted since last call to "extract" (zeroes the ticks) */
		template <class resolution>
		double extract() {
			using namespace std::chrono;

			const auto now = high_resolution_clock::now();
			double count = 0.0;

			if (is_paused) {
				count = duration_cast<duration<double, typename resolution::period>>(paused_difference).count();
			}
			else {
				const auto difference = now - ticks + paused_difference;
				count = duration_cast<duration<double, typename resolution::period>>(difference).count();
			}

			/* reset using already calculated "now" point in time */
			ticks = now;
			paused_difference = paused_difference.zero();
			return count;
		}

		/* returns time that has lasted since last call to "extract" */
		template <class resolution>
		double get() const {
			using namespace std::chrono;
			
			if (is_paused) {
				return duration_cast<duration<double, typename resolution::period>>(paused_difference).count();
			}

			const auto difference = (high_resolution_clock::now() - ticks) + paused_difference;
			return duration_cast<duration<double, typename resolution::period>>(difference).count();
		}
	};
}
