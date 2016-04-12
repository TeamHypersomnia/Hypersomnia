#pragma once
#include "misc/fixed_delta_timer.h"
#include <vector>

namespace augs {
	class renderer;

	class overworld {
		timer frame_timer;
		double delta_ms = 0.0;

	public:
		std::vector<std::pair<unsigned long long, double>> timestep_alterations_at_steps;

		fixed_delta_timer delta_timer;

		overworld();

		double delta_milliseconds() const;
		double fixed_delta_milliseconds() const;
		double delta_seconds() const;
		double view_interpolation_ratio() const;

		void assign_frame_time_to_delta_for_drawing_time_systems();
		void restore_fixed_delta();

		void configure_stepping(float fps, int max_updates_per_step);
	};
}