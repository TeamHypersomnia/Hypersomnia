#pragma once
#include "misc/delta_accumulator.h"
#include <vector>

namespace augs {
	class renderer;

	class overworld {
		timer frame_timer;
		double delta_ms = 0.0;

	public:
		struct deterministic_timer {
			overworld* parent_overworld;
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

		double delta_milliseconds();
		double delta_seconds();
		double view_interpolation_ratio();

		void assign_frame_time_to_delta();
		void restore_fixed_delta();

		void configure_stepping(float fps, int max_updates_per_step);

		deterministic_timer create_deterministic_timer();

	};
}