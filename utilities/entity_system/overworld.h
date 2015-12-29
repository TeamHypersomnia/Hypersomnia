#pragma once
#include "misc/delta_accumulator.h"
#include <vector>

namespace augs {
	class renderer;

	class overworld {
	public:
		struct deterministic_timer {
			overworld* overworld;
			unsigned long long step_recorded = 0;

			deterministic_timer(augs::overworld* owner);

			void reset();
			float get_milliseconds() const;
			float extract_milliseconds();

			unsigned get_steps() const;
			unsigned extract_steps();
		};

		std::vector<std::pair<unsigned long long, double>> timestep_alterations_at_steps;

		delta_accumulator accumulator;

		unsigned long long current_step_number = 0;

		overworld();

		void configure_stepping(float fps, int max_updates_per_step);

		deterministic_timer create_deterministic_timer();

	};
}